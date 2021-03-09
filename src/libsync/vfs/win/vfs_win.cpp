/*
Infomaniak Drive
Copyright (C) 2021 christophe.larchier@infomaniak.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "vfs_win.h"
#include "syncfileitem.h"
#include "filesystem.h"
#include "common/syncjournaldb.h"
#include "account.h"
#include "version.h"
#include "progressdispatcher.h"

// Vfs dll
#include "debug.h"
#include "vfs.h"

#include <Windows.h>
#include <iostream>
#include <unordered_map>
#include <shobjidl_core.h>

#include <QCoreApplication>
#include <QDir>
#include <QFile>

namespace KDC {

Q_LOGGING_CATEGORY(lcVfsWin, "vfs.win", QtInfoMsg)

const int s_nb_threads[NB_WORKERS] = {5, 5};

std::unordered_map<QString, OCC::SyncFileStatus::SyncFileStatusTag> s_fetchMap;

void debugCbk(TraceLevel level, const wchar_t *msg) {
    switch (level) {
    case TRACE_LEVEL_INFO:
        qCInfo(lcVfsWin) << QString::fromWCharArray(msg);
        break;
    case TRACE_LEVEL_DEBUG:
        qCDebug(lcVfsWin) << QString::fromWCharArray(msg);
        break;
    case TRACE_LEVEL_WARNING:
        qCWarning(lcVfsWin) << QString::fromWCharArray(msg);
        break;
    case TRACE_LEVEL_ERROR:
        qCCritical(lcVfsWin) << QString::fromWCharArray(msg);
        break;
    }
}

VfsWin::VfsWin(QObject *parent)
    : Vfs(parent)
{
    if (vfsInit(
                debugCbk,
                QString(APPLICATION_SHORTNAME).toStdWString().c_str(),
                OCC::Utility::escape(MIRALL_VERSION_STRING).toStdWString().c_str(),
                QString(APPLICATION_TRASH_URL).toStdWString().c_str()) != S_OK) {
        qCCritical(lcVfsWin) << "Error in vfsInit!";
        return;
    }

    // Start worker threads
    for (int i = 0; i < NB_WORKERS; i++) {
        for (int j = 0; j < s_nb_threads[i]; j++) {
            QThread *workerThread = new QThread();
            _workerInfo[i]._threadList.append(workerThread);
            Worker *worker = new Worker(this, i, j);
            worker->moveToThread(workerThread);
            connect(workerThread, &QThread::started, worker, &Worker::start);
            connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);
            connect(workerThread, &QThread::finished, workerThread, &QObject::deleteLater);
            workerThread->start();
        }
    }
}

VfsWin::~VfsWin()
{
    // Ask worker threads to stop
    for (int i = 0; i < NB_WORKERS; i++) {
        _workerInfo[i]._mutex.lock();
        _workerInfo[i]._stop = true;
        _workerInfo[i]._mutex.unlock();
        _workerInfo[i]._queueWC.wakeAll();
    }

    // Force threads to stop if needed
    for (int i = 0; i < NB_WORKERS; i++) {
        for (QThread *thread : qAsConst(_workerInfo[i]._threadList)) {
            if (thread) {
                thread->quit();
                if (!thread->wait(1000)) {
                    thread->terminate();
                    thread->wait();
                }
            }
        }
    }
}

OCC::Vfs::Mode VfsWin::mode() const
{
    return WindowsCfApi;
}

QString VfsWin::fileSuffix() const
{
    return QString();
}

void VfsWin::startImpl(const OCC::VfsSetupParams &params, QString &namespaceCLSID)
{
    qCDebug(lcVfsWin) << "startImpl - driveId = " << params.account.get()->driveId();

    wchar_t clsid[39] = L"";
    unsigned long clsidSize = sizeof(clsid);
    if (vfsStart(
                params.account.get()->driveId().toStdWString().c_str(),
                params.account.get()->davUser().toStdWString().c_str(),
                params.folderAlias.toStdWString().c_str(),
                params.folderName.toStdWString().c_str(),
                QDir::toNativeSeparators(params.filesystemPath).toStdWString().c_str(),
                clsid,
                &clsidSize) != S_OK) {
        qCCritical(lcVfsWin) << "Error in vfsStart!";
        namespaceCLSID = QString();
        return;
    }

    namespaceCLSID = QString::fromStdWString(clsid);
}

void VfsWin::dehydrate(const QString &path)
{
    qCDebug(lcVfsWin) << "dehydrate - path = " << path;

    // Dehydrate file
    if (vfsDehydratePlaceHolder(
                QDir::toNativeSeparators(path).toStdWString().c_str()) != S_OK) {
        qCCritical(lcVfsWin) << "Error in vfsDehydratePlaceHolder!";
    }

    checkAndFixMetadata(path);

    QString relativePath = path.midRef(_setupParams.filesystemPath.size()).toUtf8();
    OCC::SyncJournalFileRecord record;
    if (_setupParams.journal->getFileRecord(relativePath, &record) && record.isValid()) {
        // Unset hydrating indicator
        record._hydrating = false;
        _setupParams.journal->setFileRecord(record);
    }
}

void VfsWin::hydrate(const QString &path)
{
    qCDebug(lcVfsWin) << "hydrate - path = " << path;

    if (vfsHydratePlaceHolder(
                _setupParams.account.get()->driveId().toStdWString().c_str(),
                _setupParams.folderAlias.toStdWString().c_str(),
                QDir::toNativeSeparators(path).toStdWString().c_str()) != S_OK) {
        qCCritical(lcVfsWin) << "Error in vfsHydratePlaceHolder!";
    }

    checkAndFixMetadata(path);

    QString relativePath = path.midRef(_setupParams.filesystemPath.size()).toUtf8();
    OCC::SyncJournalFileRecord record;
    if (_setupParams.journal->getFileRecord(relativePath, &record) && record.isValid()) {
        // Unset hydrating indicator
        record._hydrating = false;
        _setupParams.journal->setFileRecord(record);
    }
}

void VfsWin::cancelHydrate(const QString &path)
{
    qCDebug(lcVfsWin) << "cancelHydrate - path = " << path;

    if (vfsCancelFetch(
                _setupParams.account.get()->driveId().toStdWString().c_str(),
                _setupParams.folderAlias.toStdWString().c_str(),
                QDir::toNativeSeparators(path).toStdWString().c_str()) != S_OK) {
        qCCritical(lcVfsWin) << "Error in vfsCancelFetch!";
        return;
    }
}

void VfsWin::exclude(const QString &path)
{
    qCDebug(lcVfsWin) << "exclude - path = " << path;

    DWORD dwAttrs = GetFileAttributesW(QDir::toNativeSeparators(path).toStdWString().c_str());
    if (dwAttrs == INVALID_FILE_ATTRIBUTES) {
        qCCritical(lcVfsWin) << "Error in GetFileAttributesW!";
        return;
    }

    if (vfsSetPinState(QDir::toNativeSeparators(path).toStdWString().c_str(), dwAttrs & FILE_ATTRIBUTE_DIRECTORY, VFS_PIN_STATE_EXCLUDED) != S_OK) {
        qCCritical(lcVfsWin) << "Error in vfsSetPinState!";
        return;
    }
}

DWORD VfsWin::getPlaceholderAttributes(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    DWORD dwParentAttr = GetFileAttributesW(fileInfo.absoluteDir().path().toStdWString().c_str());
    DWORD dwChildAttrs = 0;
    if (dwParentAttr != INVALID_FILE_ATTRIBUTES) {
        if (dwParentAttr & FILE_ATTRIBUTE_PINNED) {
            dwChildAttrs = FILE_ATTRIBUTE_PINNED;
        }
        else if (dwParentAttr & FILE_ATTRIBUTE_UNPINNED) {
            dwChildAttrs = FILE_ATTRIBUTE_UNPINNED;
        }
    }

    return dwChildAttrs;
}

void VfsWin::setPlaceholderStatus(const QString &filePath, bool directory, bool inSync)
{
    if (vfsSetPlaceHolderStatus(
                QDir::toNativeSeparators(filePath).toStdWString().c_str(),
                directory,
                inSync) != S_OK) {
        qCCritical(lcVfsWin) << "Error in vfsSetPlaceHolderStatus!";
        return;
    }
}

void VfsWin::checkAndFixMetadata(const QString &path)
{
    DWORD dwAttrs = GetFileAttributesW(QDir::toNativeSeparators(path).toStdWString().c_str());
    if (dwAttrs == INVALID_FILE_ATTRIBUTES) {
        qCCritical(lcVfsWin) << "Error in GetFileAttributesW!";
        return;
    }

    if (!(dwAttrs & FILE_ATTRIBUTE_ARCHIVE) && !(dwAttrs & FILE_ATTRIBUTE_DIRECTORY)) {
        qCCritical(lcVfsWin) << "Not a file or directory!";
        return;
    }

    bool isPlaceholder;
    bool isDehydrated;
    bool isSynced;
    if (vfsGetPlaceHolderStatus(
                QDir::toNativeSeparators(path).toStdWString().c_str(),
                &isPlaceholder,
                &isDehydrated,
                &isSynced) != S_OK) {
        qCCritical(lcVfsWin) << "Error in vfsGetPlaceHolderStatus!";
        return;
    }

    QString relativePath = path.midRef(_setupParams.filesystemPath.size()).toUtf8();
    if (dwAttrs & FILE_ATTRIBUTE_DIRECTORY) {
        // Directory

        // Update pin state in DB
        auto dbPinState = pinStateInDb(relativePath);
        auto localPinState = pinState(relativePath);
        if (*dbPinState != *localPinState) {
            if (!setPinStateInDb(relativePath, *localPinState)) {
                qCCritical(lcVfsWin) << "Error in setPinStateInDb!";
                return;
            }
        }
    }
    else if (dwAttrs & FILE_ATTRIBUTE_ARCHIVE) {
        // File

        // Update file type in DB
        ItemType type = isDehydrated ? ItemTypeVirtualFile : ItemTypeFile;
        OCC::SyncJournalFileRecord record;
        if (_setupParams.journal->getFileRecord(relativePath, &record) && record.isValid()) {
            if (record._type != type) {
                record._type = type;
                _setupParams.journal->setFileRecord(record);
            }
        }

        // Update pin state in DB
        auto dbPinState = pinStateInDb(relativePath);
        OCC::PinState localPinState = isDehydrated ? OCC::PinState::OnlineOnly : OCC::PinState::AlwaysLocal;
        if (*dbPinState != localPinState) {
            if (!setPinStateInDb(relativePath, localPinState)) {
                qCCritical(lcVfsWin) << "Error in setPinStateInDb!";
                return;
            }
        }
    }

    // Set status to synchronized
    if (isPlaceholder && !isSynced) {
        setPlaceholderStatus(path, dwAttrs & FILE_ATTRIBUTE_DIRECTORY, true);
    }
}

void VfsWin::stop()
{
    qCDebug(lcVfsWin) << "stop - driveId = " << _setupParams.account.get()->driveId();
}

void VfsWin::unregisterFolder()
{
    qCDebug(lcVfsWin) << "unregisterFolder - driveId = " << _setupParams.account.get()->driveId();

    if (vfsStop(
                _setupParams.account.get()->driveId().toStdWString().c_str(),
                _setupParams.folderAlias.toStdWString().c_str()) != S_OK) {
        qCCritical(lcVfsWin) << "Error in vfsStop!";
    }
}

bool VfsWin::isHydrating() const
{
    return false;
}

bool VfsWin::updateMetadata(const QString &filePath, time_t modtime, qint64, const QByteArray &, QString *)
{
    qCDebug(lcVfsWin) << "updateMetadata - path = " << filePath;

    OCC::FileSystem::setModTime(filePath, modtime);

    return true;
}

void VfsWin::createPlaceholder(const OCC::SyncFileItem &item)
{
    qCDebug(lcVfsWin) << "createPlaceholder - file = " << item._file;

    if (item._file.isEmpty()) {
        qCCritical(lcVfsWin) << "Empty file!";
        return;
    }

    if (item._type == ItemTypeSoftLink) {
        qCDebug(lcVfsWin) << "Cannot create a placeholder for a link file!";
        return;
    }

    if (OCC::FileSystem::fileExists(_setupParams.filesystemPath + item._file)) {
        qCWarning(lcVfsWin) << "File/directory " << item._file << " already exists!";
        return;
    }

    // Create placeholder
    WIN32_FIND_DATA findData;
    findData.nFileSizeHigh = (DWORD) (item._size >> 32);
    findData.nFileSizeLow = (DWORD) (item._size & 0xFFFFFFFF);
    OCC::Utility::UnixTimeToFiletime(item._modtime, &findData.ftLastWriteTime);
    findData.ftCreationTime = findData.ftLastWriteTime;
    findData.ftLastAccessTime = findData.ftLastWriteTime;
    findData.dwFileAttributes = getPlaceholderAttributes(_setupParams.filesystemPath + item._file);
    if (item._type == ItemTypeDirectory) {
        findData.dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
    }
    else if (item._type == ItemTypeFile) {
        findData.dwFileAttributes |= FILE_ATTRIBUTE_ARCHIVE;
    }

    if (vfsCreatePlaceHolder(
                QString(item._fileId).toStdWString().c_str(),
                QDir::toNativeSeparators(item._file).toStdWString().c_str(),
                QDir::toNativeSeparators(_setupParams.filesystemPath).toStdWString().c_str(),
                &findData) != S_OK) {
        qCCritical(lcVfsWin) << "Error in vfsCreatePlaceHolder!";
    }
}

void VfsWin::dehydratePlaceholder(const OCC::SyncFileItem &item)
{
    qCDebug(lcVfsWin) << "dehydratePlaceholder - file = " << item._file;

    if (item._file.isEmpty()) {
        qCWarning(lcVfsWin) << "No file!";
        return;
    }

    QString path(_setupParams.filesystemPath + item._file);
    if (!OCC::FileSystem::fileExists(path)) {
        // File doesn't exist
        qCCritical(lcVfsWin) << "File doesn't exist!";
        return;
    }

    // Check if the file is a placeholder
    bool isPlaceholder;
    if (vfsGetPlaceHolderStatus(
                QDir::toNativeSeparators(path).toStdWString().c_str(),
                &isPlaceholder,
                nullptr,
                nullptr) != S_OK) {
        qCCritical(lcVfsWin) << "Error in vfsGetPlaceHolderStatus!";
        return;
    }

    if (!isPlaceholder) {
        // Not a placeholder
        qCWarning(lcVfsWin) << "Not a placeholder!";
        return;
    }

    qCDebug(lcVfsWin) << "Dehydrate file " << path;
    auto dehydrateFct = [=]() {
        dehydrate(path);
    };
    std::thread dehydrateTask(dehydrateFct);
    dehydrateTask.detach();
}

bool VfsWin::convertToPlaceholder(const QString &filePath, const OCC::SyncFileItem &item)
{
    qCDebug(lcVfsWin) << "convertToPlaceholder - path = " << filePath;

    if (filePath.isEmpty()) {
        qCCritical(lcVfsWin) << "Invalid parameters";
        return false;
    }

    if (!OCC::FileSystem::fileExists(filePath)) {
        // File creation and rename
        qCDebug(lcVfsWin) << "File doesn't exist";
        return false;
    }

    QFileInfo fileInfo(filePath);
    if (fileInfo.isSymLink()) {
        qCDebug(lcVfsWin) << "Do not manage Symlink!";
        return false;
    }

    DWORD dwAttrs = GetFileAttributesW(filePath.toStdWString().c_str());
    if (dwAttrs == INVALID_FILE_ATTRIBUTES) {
        qCCritical(lcVfsWin) << "Error in GetFileAttributesW!";
        return false;
    }

    if (!(dwAttrs & FILE_ATTRIBUTE_ARCHIVE) && !(dwAttrs & FILE_ATTRIBUTE_DIRECTORY)) {
        qCDebug(lcVfsWin) << "Not a file or directory!";
        return false;
    }

    // Check if the file is already a placeholder
    bool isPlaceholder;
    if (vfsGetPlaceHolderStatus(
                QDir::toNativeSeparators(filePath).toStdWString().c_str(),
                &isPlaceholder,
                nullptr,
                nullptr) != S_OK) {
        qCCritical(lcVfsWin) << "Error in vfsGetPlaceHolderStatus!";
        return false;
    }

    if (!isPlaceholder) {
        // Convert to placeholder
        if (vfsConvertToPlaceHolder(
                    QString(item._fileId).toStdWString().c_str(),
                    QDir::toNativeSeparators(filePath).toStdWString().c_str()) != S_OK) {
            qCCritical(lcVfsWin) << "Error in vfsConvertToPlaceHolder!";
            return false;
        }
    }

    return true;
}

bool VfsWin::updateFetchStatus(const QString &tmpFilePath, const QString &filePath, qint64 received, bool &canceled)
{
    qCInfo(lcVfsWin) << "updateFetchStatus " << filePath << " - " << received;

    if (tmpFilePath.isEmpty() || filePath.isEmpty()) {
        qCCritical(lcVfsWin) << "Invalid parameters";
        return false;
    }

    if (!OCC::FileSystem::fileExists(filePath)) {
        // Download of a new file
        return true;
    }

    // Check if the file is a placeholder
    bool isPlaceholder;
    if (vfsGetPlaceHolderStatus(
                QDir::toNativeSeparators(filePath).toStdWString().c_str(),
                &isPlaceholder,
                nullptr,
                nullptr) != S_OK) {
        qCCritical(lcVfsWin) << "Error in vfsGetPlaceHolderStatus!";
        return false;
    }

    auto updateFct = [=](bool &canceled, bool &error) {
        // Update download progress
        bool finished = false;
        if (vfsUpdateFetchStatus(
                    _setupParams.account.get()->driveId().toStdWString().c_str(),
                    _setupParams.folderAlias.toStdWString().c_str(),
                    QDir::toNativeSeparators(filePath).toStdWString().c_str(),
                    QDir::toNativeSeparators(tmpFilePath).toStdWString().c_str(),
                    received,
                    &canceled,
                    &finished) != S_OK) {
            qCCritical(lcVfsWin) << "Error in vfsUpdateFetchStatus!";
            checkAndFixMetadata(filePath);
            error = true;
            return;
        }

        if (finished) {
            // Set file type to ItemTypeFile
            QString relativePath = filePath.midRef(_setupParams.filesystemPath.size()).toUtf8();
            OCC::SyncJournalFileRecord record;
            if (_setupParams.journal->getFileRecord(relativePath, &record) && record.isValid()) {
                if (record._type != ItemTypeFile) {
                    record._type = ItemTypeFile;
                    _setupParams.journal->setFileRecord(record);
                }
            }

            // Set pin state in DB
            auto dbPinState = pinStateInDb(relativePath);
            if (*dbPinState != OCC::PinState::AlwaysLocal) {
                if (!setPinStateInDb(relativePath, OCC::PinState::AlwaysLocal)) {
                    qCCritical(lcVfsWin) << "Error in setPinStateInDb!";
                    return;
                }
            }
        }
    };

    // Launch update in a separate thread
    bool error = false;
    std::thread updateTask(updateFct, std::ref(canceled), std::ref(error));
    updateTask.join();

    return !error;
}

bool VfsWin::isDehydratedPlaceholder(const QString &fileRelativePath)
{
    bool isDehydrated;
    QString filePath(_setupParams.filesystemPath + fileRelativePath);
    if (vfsGetPlaceHolderStatus(
                QDir::toNativeSeparators(filePath).toStdWString().c_str(),
                nullptr,
                &isDehydrated,
                nullptr) != S_OK) {
        qCCritical(lcVfsWin) << "Error in vfsGetPlaceHolderStatus!";
        return false;
    }

    return isDehydrated;
}

bool VfsWin::statTypeVirtualFile(csync_file_stat_t *stat, void *stat_data, const QString &fileDirectory)
{
    QString filePath(fileDirectory + stat->path);

    QFileInfo fileInfo(filePath);
    if (fileInfo.isSymLink()) {
        qCDebug(lcVfsWin) << "Status type SoftLink";
        stat->type = ItemTypeSoftLink;
        return true;
    }

    bool isPlaceholder;
    bool isDehydrated;
    bool isSynced;
    if (vfsGetPlaceHolderStatus(
                QDir::toNativeSeparators(filePath).toStdWString().c_str(),
                &isPlaceholder,
                &isDehydrated,
                &isSynced) != S_OK) {
        qCCritical(lcVfsWin) << "Error in vfsGetPlaceHolderStatus!";
        return false;
    }

    if (isPlaceholder) {
        DWORD dwAttrs = GetFileAttributesW(filePath.toStdWString().c_str());
        if (dwAttrs == INVALID_FILE_ATTRIBUTES) {
            qCCritical(lcVfsWin) << "Error in GetFileAttributesW!";
            return false;
        }

        if (!(dwAttrs & FILE_ATTRIBUTE_ARCHIVE) && !(dwAttrs & FILE_ATTRIBUTE_DIRECTORY)) {
            qCCritical(lcVfsWin) << "Not a file or directory!";
            return false;
        }

        if (dwAttrs & FILE_ATTRIBUTE_DIRECTORY) {
            // Directory
            stat->type = ItemTypeDirectory;
            return true;
        }
        else if (dwAttrs & FILE_ATTRIBUTE_ARCHIVE) {
            // File
            WIN32_FIND_DATA *ffd = (WIN32_FIND_DATA *) stat_data;
            if (ffd && ffd->dwFileAttributes != INVALID_FILE_ATTRIBUTES) {
                if ((ffd->dwFileAttributes & FILE_ATTRIBUTE_OFFLINE) && (ffd->dwFileAttributes & FILE_ATTRIBUTE_PINNED)) {
                    QString fileRelativePath = fileInfo.canonicalFilePath().midRef(_setupParams.filesystemPath.size()).toUtf8();
                    OCC::SyncJournalFileRecord record;
                    if (_setupParams.journal->getFileRecord(fileRelativePath, &record) && record.isValid()) {
                        if (record._hydrating) {
                            // Hydration in progress
                            stat->type = ItemTypeVirtualFile;
                        }
                        else {
                            stat->type = ItemTypeVirtualFileDownload;
                        }
                    }

                    return true;
                }
                else if (!(ffd->dwFileAttributes & FILE_ATTRIBUTE_OFFLINE) && (ffd->dwFileAttributes & FILE_ATTRIBUTE_UNPINNED)) {
                    stat->type = ItemTypeVirtualFileDehydration;
                    return true;
                }
            }

            if (isDehydrated) {
                stat->type = ItemTypeVirtualFile;
                return true;
            }
        }
    }

    return false;
}

bool VfsWin::setPinState(const QString &fileRelativePath, OCC::PinState state)
{
    // Write pin state to database
    return setPinStateInDb(fileRelativePath, state);
}

OCC::Optional<OCC::PinState> VfsWin::pinState(const QString &relativePath)
{
    // Read pin state from file attributes
    QString path(_setupParams.filesystemPath + relativePath);
    DWORD dwAttrs = GetFileAttributesW(path.toStdWString().c_str());

    if (dwAttrs != INVALID_FILE_ATTRIBUTES) {
        if (dwAttrs & FILE_ATTRIBUTE_PINNED) {
            return OCC::PinState::AlwaysLocal;
        }
        else if (dwAttrs & FILE_ATTRIBUTE_UNPINNED) {
            return OCC::PinState::OnlineOnly;
        }
    }

    return OCC::PinState::Unspecified;
}

OCC::Vfs::AvailabilityResult VfsWin::availability(const QString &fileRelativePath)
{
    return availabilityInDb(fileRelativePath);
}

void VfsWin::fileStatusChanged(const QString &path, OCC::SyncFileStatus status)
{
    qCDebug(lcVfsWin) << "fileStatusChanged - path = " << path << " - status = " << status.tag();

    if (!OCC::FileSystem::fileExists(path)) {
        // New file
        return;
    }

    if (status.tag() == OCC::SyncFileStatus::StatusExcluded) {
        exclude(path);
    }
    else if (status.tag() == OCC::SyncFileStatus::StatusUpToDate) {
        checkAndFixMetadata(path);
    }
    else if (status.tag() == OCC::SyncFileStatus::StatusSync) {
        if (!QDir(path).exists()) {
            // File
            QString fileRelativePath = path.midRef(_setupParams.filesystemPath.size()).toUtf8();
            auto localPinState = pinState(fileRelativePath);
            bool isDehydrated = isDehydratedPlaceholder(fileRelativePath);
            if (*localPinState == OCC::PinState::OnlineOnly && !isDehydrated) {
                // Add file path to dehydration queue
                _workerInfo[WORKER_DEHYDRATION]._mutex.lock();
                _workerInfo[WORKER_DEHYDRATION]._queue.push_front(path);
                _workerInfo[WORKER_DEHYDRATION]._mutex.unlock();
                _workerInfo[WORKER_DEHYDRATION]._queueWC.wakeOne();
            }
            else if (*localPinState == OCC::PinState::AlwaysLocal && isDehydrated) {
                OCC::SyncJournalFileRecord record;
                if (_setupParams.journal->getFileRecord(fileRelativePath, &record) && record.isValid()) {
                    if (!record._hydrating) {
                        // Set hydrating indicator (avoid double hydration)
                        record._hydrating = true;
                        _setupParams.journal->setFileRecord(record);

                        // Add file path to hydration queue
                        _workerInfo[WORKER_HYDRATION]._mutex.lock();
                        _workerInfo[WORKER_HYDRATION]._queue.push_front(path);
                        _workerInfo[WORKER_HYDRATION]._mutex.unlock();
                        _workerInfo[WORKER_HYDRATION]._queueWC.wakeOne();
                    }
                }
            }
        }
    }
    else if (status.tag() == OCC::SyncFileStatus::StatusWarning ||
            status.tag() == OCC::SyncFileStatus::StatusError) {
        // Nothing to do
    }
}

Worker::Worker(VfsWin *vfs, int type, int num)
    : _vfs(vfs)
    , _type(type)
    , _num(num)
{
}

void Worker::start()
{
    qCDebug(lcVfsWin) << "Worker" << _type << _num << "started";

    WorkerInfo &workerInfo = _vfs->_workerInfo[_type];

    forever {
        workerInfo._mutex.lock();
        while (workerInfo._queue.empty() && !workerInfo._stop) {
            qCDebug(lcVfsWin) << "Worker" << _type << _num << "waiting";
            workerInfo._queueWC.wait(&workerInfo._mutex);
        }

        if (workerInfo._stop) {
            workerInfo._mutex.unlock();
            break;
        }

        QString path = workerInfo._queue.back();
        workerInfo._queue.pop_back();
        workerInfo._mutex.unlock();

        qCDebug(lcVfsWin) << "Worker" << _type << _num << "working";

        switch (_type) {
        case WORKER_HYDRATION:
            _vfs->hydrate(path);
            break;
        case WORKER_DEHYDRATION:
            _vfs->dehydrate(path);
            break;
        }
    }

    qCDebug(lcVfsWin) << "Worker" << _type << _num << "ended";
}

} // namespace OCC
