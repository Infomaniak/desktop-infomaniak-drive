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

#include "CloudFileProviderDll.h"

#include <Windows.h>
#include <iostream>
#include <thread>
#include <unordered_map>

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

void VfsWin::startImpl(const OCC::VfsSetupParams &params)
{
    qCDebug(lcVfsWin) << "startImpl - Begin - driveId = " << params.account.get()->driveId();

    if (CFPInitCloudFileProvider(debugCbk, QString(APPLICATION_SHORTNAME).toStdWString().c_str()) != S_OK) {
        qCCritical(lcVfsWin) << "Error in CFPInitCloudFileProvider!";
        return;
    }

    if (CFPStartCloudFileProvider(
                params.account.get()->driveId().toStdWString().c_str(),
                params.providerName.toStdWString().c_str(),
                params.account.get()->davUser().toStdWString().c_str(),
                QDir::toNativeSeparators(params.filesystemPath).toStdWString().c_str()) != S_OK) {
        qCCritical(lcVfsWin) << "Error in CFPStartCloudFileProvider!";
    }

    qCDebug(lcVfsWin) << "startImpl - End";
}

void VfsWin::dehydrateFile(const QString &filePath)
{
    qCDebug(lcVfsWin) << "dehydrateFile - Begin - path = " << filePath;

    // Dehydrate file
    if (CFPDehydratePlaceHolder(
            _setupParams.account.get()->driveId().toStdWString().c_str(),
            QDir::toNativeSeparators(filePath).toStdWString().c_str())) {
        qCCritical(lcVfsWin) << "Error in CFPDehydratePlaceHolder!";
        return;
    }
    // Update file type in DB
    QString fileRelativePath = filePath.midRef(_setupParams.filesystemPath.size()).toUtf8();
    OCC::SyncJournalFileRecord record;
    if (_setupParams.journal->getFileRecord(fileRelativePath, &record) && record.isValid()) {
        record._type = ItemTypeVirtualFile;
        _setupParams.journal->setFileRecord(record);
    }
    // Update pin state in DB
    setPinStateInDb(fileRelativePath, OCC::PinState::OnlineOnly);

    qCDebug(lcVfsWin) << "dehydrateFile - End";
}

void VfsWin::hydrateFile(const QString &filePath)
{
    qCDebug(lcVfsWin) << "hydrateFile - Begin - path = " << filePath;

    if (CFPHydratePlaceHolder(
            _setupParams.account.get()->driveId().toStdWString().c_str(),
            QDir::toNativeSeparators(filePath).toStdWString().c_str())) {
        qCCritical(lcVfsWin) << "Error in CFPHydratePlaceHolder!";
        return;
    }

    qCDebug(lcVfsWin) << "hydrateFile - End";
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

void VfsWin::stop()
{
    qCDebug(lcVfsWin) << "stop";
}

void VfsWin::unregisterFolder()
{
    qCDebug(lcVfsWin) << "unregisterFolder - Begin - driveId = " << _setupParams.account.get()->driveId();

    if (CFPStopCloudFileProvider(_setupParams.account.get()->driveId().toStdWString().c_str()) != S_OK) {
        qCCritical(lcVfsWin) << "Error in CFPStopCloudFileProvider!";
    }

    qCDebug(lcVfsWin) << "unregisterFolder - End";
}

bool VfsWin::isHydrating() const
{
    return false;
}

bool VfsWin::updateMetadata(const QString &filePath, time_t modtime, qint64, const QByteArray &, QString *)
{
    qCDebug(lcVfsWin) << "updateMetadata - Begin - path = " << filePath;

    OCC::FileSystem::setModTime(filePath, modtime);

    qCDebug(lcVfsWin) << "updateMetadata - End";
    return true;
}

void VfsWin::createPlaceholder(const OCC::SyncFileItem &item)
{
    qCDebug(lcVfsWin) << "createPlaceholder - Begin - file = " << item._file;

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
    wcscpy_s(findData.cFileName, MAX_PATH, QDir::toNativeSeparators(item._file).toStdWString().c_str());
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

    if (CFPCreatePlaceHolder(
                QString(item._fileId).toStdWString().c_str(),
                QDir::toNativeSeparators(_setupParams.filesystemPath).toStdWString().c_str(),
                &findData) != S_OK) {
        qCCritical(lcVfsWin) << "Error in CFPCreatePlaceHolder!";
    }

    qCDebug(lcVfsWin) << "createPlaceholder - End";
}

void VfsWin::dehydratePlaceholder(const OCC::SyncFileItem &item)
{
    qCDebug(lcVfsWin) << "dehydratePlaceholder - Begin - file = " << item._file;

    if (item._file.isEmpty()) {
        qCWarning(lcVfsWin) << "Empty file!";
        return;
    }

    // Check if the file is a placeholder
    if (!OCC::FileSystem::fileExists(_setupParams.filesystemPath + item._file)) {
        // File doesn't exist
        qCWarning(lcVfsWin) << "File doesn't exist!";
        return;
    }

    bool isPlaceholder;
    if (CFPGetPlaceHolderStatus(
                QDir::toNativeSeparators(_setupParams.filesystemPath).toStdWString().c_str(),
                QDir::toNativeSeparators(item._file).toStdWString().c_str(),
                &isPlaceholder,
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

    dehydrateFile(_setupParams.filesystemPath + item._file);

    qCDebug(lcVfsWin) << "dehydratePlaceholder - End";
}

bool VfsWin::convertToPlaceholder(const QString &filePath, const OCC::SyncFileItem &item, const QString &replacesFile)
{
    qCDebug(lcVfsWin) << "convertToPlaceholder - Begin - path = " << filePath;

    if (!OCC::FileSystem::fileExists(_setupParams.filesystemPath + item._file)) {
        qCCritical(lcVfsWin) << "File doesn't exist";
        return false;
    }

    // Check if file is already a placeholder
    bool isPlaceholder;
    if (CFPGetPlaceHolderStatus(
                QDir::toNativeSeparators(_setupParams.filesystemPath).toStdWString().c_str(),
                QDir::toNativeSeparators(item._file).toStdWString().c_str(),
                &isPlaceholder,
                nullptr,
                nullptr) != S_OK) {
        qCCritical(lcVfsWin) << "Error in CFPGetPlaceHolderStatus!";
        return false;
    }

    if (!isPlaceholder) {
        // Convert to placeholder
        WIN32_FIND_DATA findData;
        wcscpy_s(findData.cFileName, MAX_PATH, QDir::toNativeSeparators(item._file).toStdWString().c_str());
        findData.dwFileAttributes = getPlaceholderAttributes(_setupParams.filesystemPath + item._file);
        if (item._type == ItemTypeDirectory) {
            findData.dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
        }
        else {
            findData.dwFileAttributes |= FILE_ATTRIBUTE_ARCHIVE;
        }

        if (CFPConvertToPlaceHolder(
                    QString(item._fileId).toStdWString().c_str(),
                    QDir::toNativeSeparators(_setupParams.filesystemPath).toStdWString().c_str(),
                    &findData) != S_OK) {
            qCCritical(lcVfsWin) << "Error in CFPConvertToPlaceHolder!";
            return false;
        }
    }

    if (!replacesFile.isEmpty()) {
        // Download finished
        if (CFPUpdateFetchStatus(
                    _setupParams.account.get()->driveId().toStdWString().c_str(),
                    QDir::toNativeSeparators(replacesFile).toStdWString().c_str(),
                    QDir::toNativeSeparators(filePath).toStdWString().c_str(),
                    item._size) != S_OK) {
            qCCritical(lcVfsWin) << "Error in CFPUpdateFetchStatus!";
            return false;
        }
    }

    qCDebug(lcVfsWin) << "convertToPlaceholder - End";
    return true;
}

bool VfsWin::isDehydratedPlaceholder(const QString &fileRelativePath)
{
    qCDebug(lcVfsWin) << "isDehydratedPlaceholder - Begin - path = " << fileRelativePath;

    bool isDehydrated;
    if (CFPGetPlaceHolderStatus(
                QDir::toNativeSeparators(_setupParams.filesystemPath).toStdWString().c_str(),
                fileRelativePath.toStdWString().c_str(),
                nullptr,
                &isDehydrated,
                nullptr) != S_OK) {
        qCCritical(lcVfsWin) << "Error in CFPGetPlaceHolderStatus!";
        return false;
    }

    qCDebug(lcVfsWin) << "isDehydratedPlaceholder - End";
    return isDehydrated;
}

bool VfsWin::statTypeVirtualFile(csync_file_stat_t *stat, void *stat_data, const QString &fileDirectory)
{
    Q_UNUSED(stat_data)

    qCDebug(lcVfsWin) << "statTypeVirtualFile - Begin - path = " << stat->path;

    bool isPlaceholder;
    bool isDehydrated;
    bool isDirectory;
    if (CFPGetPlaceHolderStatus(
                QDir::toNativeSeparators(fileDirectory).toStdWString().c_str(),
                QDir::toNativeSeparators(stat->path).toStdWString().c_str(),
                &isPlaceholder,
                &isDehydrated,
                &isDirectory) != S_OK) {
        qCCritical(lcVfsWin) << "Error in CFPGetPlaceHolderStatus!";
        return false;
    }

    if (isPlaceholder) {
        if (isDehydrated) {
            stat->type = ItemTypeVirtualFile;
            qCDebug(lcVfsWin) << "Status type VirtualFile";
            return true;
        }

        if (isDirectory) {
            stat->type = ItemTypeDirectory;
            qCDebug(lcVfsWin) << "Status type Directory";
            return true;
        }
    }

    qCDebug(lcVfsWin) << "statTypeVirtualFile - End";
    return false;
}

bool VfsWin::setPinState(const QString &fileRelativePath, OCC::PinState state)
{
    // Write pin state to database
    QString filePath(_setupParams.filesystemPath + fileRelativePath);
    return setPinStateInDb(fileRelativePath, state);
}

OCC::Optional<OCC::PinState> VfsWin::pinState(const QString &fileRelativePath)
{
    // Read pin state from file attributes
    QString filePath(_setupParams.filesystemPath + fileRelativePath);
    DWORD dwAttrs = GetFileAttributesW(filePath.toStdWString().c_str());

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

void VfsWin::fileStatusChanged(const QString &filePath, OCC::SyncFileStatus status)
{
    qCDebug(lcVfsWin) << "fileStatusChanged - Begin - path = " << filePath;

    if (!OCC::FileSystem::fileExists(filePath)) {
        qCCritical(lcVfsWin) << "File doesn't exist";
        return;
    }

    if (QDir(filePath).exists()) {
        qCDebug(lcVfsWin) << "Directory - Nothing to do";
        return;
    }

    if (status.tag() == OCC::SyncFileStatus::StatusExcluded) {
        qCDebug(lcVfsWin) << "Status Excluded";
        if (CFPSetPinStateExcluded(QDir::toNativeSeparators(filePath).toStdWString().c_str())) {
            qCCritical(lcVfsWin) << "Error in CFPSetPinStateExcluded!";
            return;
        }
    }
    else if (status.tag() == OCC::SyncFileStatus::StatusUpToDate) {
        qCDebug(lcVfsWin) << "Status UpToDate";
    }
    else if (status.tag() == OCC::SyncFileStatus::StatusSync) {
        QString fileRelativePath = filePath.midRef(_setupParams.filesystemPath.size()).toUtf8();
        auto localPinState = pinState(fileRelativePath);
        auto dbPinState = pinStateInDb(fileRelativePath);
        bool isDehydrated = isDehydratedPlaceholder(fileRelativePath);
        if (*localPinState == OCC::PinState::OnlineOnly && !isDehydrated) {
            qCDebug(lcVfsWin) << "Dehydrate file " << filePath;
            dehydrateFile(filePath);
        }
        else if (*localPinState == OCC::PinState::AlwaysLocal && isDehydrated) {
            qCDebug(lcVfsWin) << "Hydrate file " << filePath;
            auto hydrateFct = [=]() {
                hydrateFile(filePath);
            };
            std::thread hydrateTask(hydrateFct);
            hydrateTask.detach();
        }
        else if (*dbPinState != *localPinState) {
            qCDebug(lcVfsWin) << "Fix DB type and pin state";
            // Update file type in DB
            OCC::SyncJournalFileRecord record;
            if (_setupParams.journal->getFileRecord(fileRelativePath, &record) && record.isValid()) {
                record._type = isDehydrated ? ItemTypeVirtualFile : ItemTypeFile;
                _setupParams.journal->setFileRecord(record);
            }
            // Update pin state in DB
            setPinStateInDb(fileRelativePath, isDehydrated ? OCC::PinState::OnlineOnly : OCC::PinState::AlwaysLocal);
        }
    }
    else if (status.tag() == OCC::SyncFileStatus::StatusWarning ||
            status.tag() == OCC::SyncFileStatus::StatusError) {
        qCDebug(lcVfsWin) << "Cancel hydration of file " << filePath;
        if (CFPCancelFetch(
                    _setupParams.account.get()->driveId().toStdWString().c_str(),
                    QDir::toNativeSeparators(filePath).toStdWString().c_str()) != S_OK) {
            qCWarning(lcVfsWin) << "Error in CFPCancelFetch!";
            return;
        }
    }

    qCDebug(lcVfsWin) << "fileStatusChanged - End";
}

} // namespace OCC
