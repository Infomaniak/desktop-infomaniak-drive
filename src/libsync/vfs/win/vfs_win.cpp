/*
Infomaniak Drive
Copyright (C) 2020 christophe.larchier@infomaniak.com

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

// CloudFileProvider dll
#include "debug.h"
#include "cloudfileproviderdll.h"

#include <Windows.h>
#include <iostream>
#include <thread>
#include <unordered_map>
#include <shobjidl_core.h>

#include <QFile>
#include <QDir>

namespace KDC {

Q_LOGGING_CATEGORY(lcVfsWin, "vfs.win", QtInfoMsg)

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
    if (cfpInitCloudFileProvider(
                debugCbk,
                QString(APPLICATION_SHORTNAME).toStdWString().c_str(),
                OCC::Utility::escape(MIRALL_VERSION_STRING).toStdWString().c_str(),
                QString(APPLICATION_TRASH_URL).toStdWString().c_str()) != S_OK) {
        qCCritical(lcVfsWin) << "Error in CFPInitCloudFileProvider!";
        return;
    }
}

VfsWin::~VfsWin()
{
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
    if (cfpStartCloudFileProvider(
                params.account.get()->driveId().toStdWString().c_str(),
                params.account.get()->davUser().toStdWString().c_str(),
                params.folderAlias.toStdWString().c_str(),
                params.folderName.toStdWString().c_str(),
                QDir::toNativeSeparators(params.filesystemPath).toStdWString().c_str(),
                clsid,
                &clsidSize) != S_OK) {
        qCCritical(lcVfsWin) << "Error in CFPStartCloudFileProvider!";
    }

    namespaceCLSID = QString::fromStdWString(clsid);
}

void VfsWin::dehydrate(const QString &path)
{
    qCDebug(lcVfsWin) << "dehydrate - path = " << path;

    // Dehydrate file
    if (cfpDehydratePlaceHolder(
                _setupParams.account.get()->driveId().toStdWString().c_str(),
                _setupParams.folderAlias.toStdWString().c_str(),
                QDir::toNativeSeparators(path).toStdWString().c_str()) != S_OK) {
        qCCritical(lcVfsWin) << "Error in CFPDehydratePlaceHolder!";
        return;
    }
}

void VfsWin::hydrate(const QString &path)
{
    qCDebug(lcVfsWin) << "hydrate - path = " << path;

    if (cfpHydratePlaceHolder(
                _setupParams.account.get()->driveId().toStdWString().c_str(),
                _setupParams.folderAlias.toStdWString().c_str(),
                QDir::toNativeSeparators(path).toStdWString().c_str()) != S_OK) {
        qCCritical(lcVfsWin) << "Error in CFPHydratePlaceHolder!";
        return;
    }
}

void VfsWin::cancelHydrate(const QString &path)
{
    qCDebug(lcVfsWin) << "cancelHydrate - path = " << path;

    if (cfpCancelFetch(
                _setupParams.account.get()->driveId().toStdWString().c_str(),
                _setupParams.folderAlias.toStdWString().c_str(),
                QDir::toNativeSeparators(path).toStdWString().c_str()) != S_OK) {
        qCWarning(lcVfsWin) << "Error in CFPCancelFetch!";
        return;
    }
}

void VfsWin::exclude(const QString &path)
{
    qCDebug(lcVfsWin) << "exclude - path = " << path;

    bool isDirectory;
    if (cfpGetPlaceHolderStatus(
                QDir::toNativeSeparators(path).toStdWString().c_str(),
                nullptr,
                nullptr,
                nullptr,
                &isDirectory) != S_OK) {
        qCCritical(lcVfsWin) << "Error in CFPGetPlaceHolderStatus!";
        return;
    }

    if (cfpSetPinState(QDir::toNativeSeparators(path).toStdWString().c_str(), isDirectory, CFP_PIN_STATE_EXCLUDED) != S_OK) {
        qCCritical(lcVfsWin) << "Error in CFPSetPinState!";
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
    if (cfpSetPlaceHolderStatus(
                QDir::toNativeSeparators(filePath).toStdWString().c_str(),
                directory,
                inSync) != S_OK) {
        qCCritical(lcVfsWin) << "Error in CFPSetPlaceHolderStatus!";
        return;
    }
}

void VfsWin::checkAndFixMetadata(const QString &path)
{
    bool isPlaceholder;
    bool isDehydrated;
    bool isSynced;
    bool isDirectory;
    if (cfpGetPlaceHolderStatus(
                QDir::toNativeSeparators(path).toStdWString().c_str(),
                &isPlaceholder,
                &isDehydrated,
                &isSynced,
                &isDirectory) != S_OK) {
        qCCritical(lcVfsWin) << "Error in CFPGetPlaceHolderStatus!";
        return;
    }

    QString relativePath = path.midRef(_setupParams.filesystemPath.size()).toUtf8();
    if (isDirectory) {
        // Update pin state in DB
        auto dbPinState = pinStateInDb(relativePath);
        auto localPinState = pinState(relativePath);
        if (*dbPinState != *localPinState) {
            setPinStateInDb(relativePath, *localPinState);
        }
    }
    else {
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
            setPinStateInDb(relativePath, localPinState);
        }
    }

    // Set status to synchronized
    if (isPlaceholder && !isSynced) {
        setPlaceholderStatus(path, isDirectory, true);
    }
}

void VfsWin::stop()
{
    qCDebug(lcVfsWin) << "stop";
}

void VfsWin::unregisterFolder()
{
    qCDebug(lcVfsWin) << "unregisterFolder - driveId = " << _setupParams.account.get()->driveId();

    if (cfpStopCloudFileProvider(
                _setupParams.account.get()->driveId().toStdWString().c_str(),
                _setupParams.folderAlias.toStdWString().c_str()) != S_OK) {
        qCCritical(lcVfsWin) << "Error in CFPStopCloudFileProvider!";
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
        qCWarning(lcVfsWin) << "Empty file!";
        return;
    }

    if (OCC::FileSystem::fileExists(_setupParams.filesystemPath + item._file)) {
        qCWarning(lcVfsWin) << "File/directory " << item._file << " already exists!";
        return;
    }

    // Create placeholder
    WIN32_FIND_DATA findData;
    findData.nFileSizeHigh = (DWORD) (item._size & 0xFFFFFFFF00000000);
    findData.nFileSizeLow = (DWORD) (item._size);
    OCC::Utility::UnixTimeToFiletime(item._modtime, &findData.ftLastWriteTime);
    findData.ftCreationTime = findData.ftLastWriteTime;
    findData.ftLastAccessTime = findData.ftLastWriteTime;
    findData.dwFileAttributes = getPlaceholderAttributes(_setupParams.filesystemPath + item._file);
    if (item._type == ItemTypeDirectory) {
        findData.dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
    }
    else {
        findData.dwFileAttributes |= FILE_ATTRIBUTE_ARCHIVE;
    }

    if (cfpCreatePlaceHolder(
                QDir::toNativeSeparators(item._file).toStdWString().c_str(),
                QDir::toNativeSeparators(_setupParams.filesystemPath).toStdWString().c_str(),
                &findData) != S_OK) {
        qCCritical(lcVfsWin) << "Error in CFPCreatePlaceHolder!";
    }
}

void VfsWin::dehydratePlaceholder(const OCC::SyncFileItem &item)
{
    qCDebug(lcVfsWin) << "dehydratePlaceholder - file = " << item._file;

    if (item._file.isEmpty()) {
        qCWarning(lcVfsWin) << "Empty file!";
        return;
    }

    // Check if the file is a placeholder
    QString path(_setupParams.filesystemPath + item._file);
    if (!OCC::FileSystem::fileExists(path)) {
        // File doesn't exist
        qCWarning(lcVfsWin) << "File doesn't exist!";
        return;
    }

    bool isPlaceholder;
    if (cfpGetPlaceHolderStatus(
                QDir::toNativeSeparators(path).toStdWString().c_str(),
                &isPlaceholder,
                nullptr,
                nullptr,
                nullptr) != S_OK) {
        qCCritical(lcVfsWin) << "Error in CFPGetPlaceHolderStatus!";
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

bool VfsWin::convertToPlaceholder(const QString &filePath, const OCC::SyncFileItem &item, const QString &replacesFile)
{
    qCDebug(lcVfsWin) << "convertToPlaceholder - path = " << filePath;

    if (!OCC::FileSystem::fileExists(filePath)) {
        qCCritical(lcVfsWin) << "File doesn't exist";
        return false;
    }

    // Check if file is already a placeholder
    bool isPlaceholder;
    bool isSynced;
    bool isDirectory;
    if (cfpGetPlaceHolderStatus(
                QDir::toNativeSeparators(filePath).toStdWString().c_str(),
                &isPlaceholder,
                nullptr,
                &isSynced,
                &isDirectory) != S_OK) {
        qCCritical(lcVfsWin) << "Error in CFPGetPlaceHolderStatus!";
        return false;
    }

    if (!isPlaceholder) {
        // Convert to placeholder
        if (cfpConvertToPlaceHolder(
                    QString(item._fileId).toStdWString().c_str(),
                    QDir::toNativeSeparators(filePath).toStdWString().c_str()) != S_OK) {
            qCCritical(lcVfsWin) << "Error in CFPConvertToPlaceHolder!";
            return false;
        }
    }

    if (!replacesFile.isEmpty()) {
        // Download finished
        if (cfpUpdateFetchStatus(
                    _setupParams.account.get()->driveId().toStdWString().c_str(),
                    _setupParams.folderAlias.toStdWString().c_str(),
                    QDir::toNativeSeparators(replacesFile).toStdWString().c_str(),
                    QDir::toNativeSeparators(filePath).toStdWString().c_str(),
                    item._size) != S_OK) {
            qCCritical(lcVfsWin) << "Error in CFPUpdateFetchStatus!";
            return false;
        }

        // Force pin state to pinned
        if (cfpSetPinState(QDir::toNativeSeparators(filePath).toStdWString().c_str(), isDirectory, CFP_PIN_STATE_PINNED)) {
            qCCritical(lcVfsWin) << "Error in CFPSetPinState!";
            return false;
        }
    }

    return true;
}

bool VfsWin::isDehydratedPlaceholder(const QString &fileRelativePath)
{
    qCDebug(lcVfsWin) << "isDehydratedPlaceholder - path = " << fileRelativePath;

    bool isDehydrated;
    QString filePath(_setupParams.filesystemPath + fileRelativePath);
    if (cfpGetPlaceHolderStatus(
                QDir::toNativeSeparators(filePath).toStdWString().c_str(),
                nullptr,
                &isDehydrated,
                nullptr,
                nullptr) != S_OK) {
        qCCritical(lcVfsWin) << "Error in CFPGetPlaceHolderStatus!";
        return false;
    }

    return isDehydrated;
}

bool VfsWin::statTypeVirtualFile(csync_file_stat_t *stat, void *stat_data, const QString &fileDirectory)
{
    qCDebug(lcVfsWin) << "statTypeVirtualFile - path = " << stat->path;

    QString filePath(fileDirectory + stat->path);
    bool isPlaceholder;
    bool isDehydrated;
    bool isSynced;
    bool isDirectory;
    if (cfpGetPlaceHolderStatus(
                QDir::toNativeSeparators(filePath).toStdWString().c_str(),
                &isPlaceholder,
                &isDehydrated,
                &isSynced,
                &isDirectory) != S_OK) {
        qCCritical(lcVfsWin) << "Error in CFPGetPlaceHolderStatus!";
        return false;
    }

    if (isPlaceholder) {
        if (isDirectory) {
            stat->type = ItemTypeDirectory;
            qCDebug(lcVfsWin) << "Status type Directory";
            return true;
        }
        else {
            WIN32_FIND_DATA *ffd = (WIN32_FIND_DATA *) stat_data;
            if (ffd && ffd->dwFileAttributes != INVALID_FILE_ATTRIBUTES) {
                if ((ffd->dwFileAttributes & FILE_ATTRIBUTE_OFFLINE) && (ffd->dwFileAttributes & FILE_ATTRIBUTE_PINNED)) {
                    qCDebug(lcVfsWin) << "Status type VirtualFileDownload";
                    stat->type = ItemTypeVirtualFileDownload;
                    return true;
                }
                else if (!(ffd->dwFileAttributes & FILE_ATTRIBUTE_OFFLINE) && (ffd->dwFileAttributes & FILE_ATTRIBUTE_UNPINNED)) {
                    qCDebug(lcVfsWin) << "Status type VirtualFileDehydration";
                    stat->type = ItemTypeVirtualFileDehydration;
                    return true;
                }
            }

            if (isDehydrated) {
                qCDebug(lcVfsWin) << "Status type VirtualFile";
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
    QString filePath(_setupParams.filesystemPath + fileRelativePath);
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
                qCDebug(lcVfsWin) << "Dehydrate file " << path;
                auto dehydrateFct = [=]() {
                    dehydrate(path);
                };
                std::thread dehydrateTask(dehydrateFct);
                dehydrateTask.detach();
            }
            else if (*localPinState == OCC::PinState::AlwaysLocal && isDehydrated) {
                qCDebug(lcVfsWin) << "Hydrate file " << path;
                auto hydrateFct = [=]() {
                    hydrate(path);
                };
                std::thread hydrateTask(hydrateFct);
                hydrateTask.detach();
            }
        }
    }
    else if (status.tag() == OCC::SyncFileStatus::StatusWarning ||
            status.tag() == OCC::SyncFileStatus::StatusError) {
        if (!QDir(path).exists()) {
            // File
            qCDebug(lcVfsWin) << "Cancel hydration of file " << path;
            cancelHydrate(path);
        }
    }
}

} // namespace OCC
