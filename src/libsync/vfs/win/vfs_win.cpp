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

#include <windows.h>
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
    qCDebug(lcVfsWin) << "Begin";

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
}

void VfsWin::stop()
{
}

void VfsWin::unregisterFolder()
{
    if (CFPStopCloudFileProvider(_setupParams.account.get()->driveId().toStdWString().c_str()) != S_OK) {
        qCWarning(lcVfsWin) << "Error in CFPStopCloudFileProvider!";
    }
}

bool VfsWin::isHydrating() const
{
    return false;
}

bool VfsWin::updateMetadata(const QString &filePath, time_t modtime, qint64, const QByteArray &, QString *)
{
    OCC::FileSystem::setModTime(filePath, modtime);
    return true;
}

void VfsWin::createPlaceholder(const OCC::SyncFileItem &item)
{
    if (item._file.isEmpty()) {
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
    findData.dwFileAttributes = FILE_ATTRIBUTE_UNPINNED;
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
        qCWarning(lcVfsWin) << "Error in CFPCreatePlaceHolder!";
    }
}

void VfsWin::dehydratePlaceholder(const OCC::SyncFileItem &item)
{
    if (item._file.isEmpty()) {
        return;
    }

    // Check if file is a placeholder
    if (OCC::FileSystem::fileExists(_setupParams.filesystemPath + item._file)) {
        bool isPlaceholder;
        if (CFPGetPlaceHolderStatus(
                    QDir::toNativeSeparators(_setupParams.filesystemPath).toStdWString().c_str(),
                    QDir::toNativeSeparators(item._file).toStdWString().c_str(),
                    &isPlaceholder,
                    nullptr,
                    nullptr) != S_OK) {
            qCWarning(lcVfsWin) << "Error in CFPGetPlaceHolderStatus!";
            return;
        }

        if (!isPlaceholder) {
            // Not a placeholder
            return;
        }
    }
    else {
        return;
    }

    if (CFPDehydratePlaceHolder(
            QDir::toNativeSeparators(_setupParams.filesystemPath).toStdWString().c_str(),
            item._file.toStdWString().c_str())) {
        qCWarning(lcVfsWin) << "Error in CFPDehydratePlaceHolder!";
        return;
    }

    /*
    // Move the item's pin state
    auto pin = _setupParams.journal->internalPinStates().rawForPath(item._file.toUtf8());
    if (pin && *pin != OCC::PinState::Inherited) {
        setPinState(item._renameTarget, *pin);
        setPinState(item._file, OCC::PinState::Inherited);
    }

    // Ensure the pin state isn't contradictory
    pin = pinState(item._renameTarget);
    if (pin && *pin == OCC::PinState::AlwaysLocal)
        setPinState(item._renameTarget, OCC::PinState::Unspecified);
    */
}

bool VfsWin::convertToPlaceholder(const QString &filePath, const OCC::SyncFileItem &item, const QString &replacesFile)
{
    if (!OCC::FileSystem::fileExists(_setupParams.filesystemPath + item._file)) {
        qCWarning(lcVfsWin) << "File/directory " << item._file << " doesn't exist!";
        return false;
    }

    if (replacesFile.isEmpty()) {
        // Check if file is already a placeholder
        bool isPlaceholder;
        if (CFPGetPlaceHolderStatus(
                    QDir::toNativeSeparators(_setupParams.filesystemPath).toStdWString().c_str(),
                    QDir::toNativeSeparators(item._file).toStdWString().c_str(),
                    &isPlaceholder,
                    nullptr,
                    nullptr) != S_OK) {
            qCWarning(lcVfsWin) << "Error in CFPGetPlaceHolderStatus!";
            return false;
        }

        if (isPlaceholder) {
            // Already placeholder
            return true;
        }

        // Convert to placeholder
        WIN32_FIND_DATA findData;
        wcscpy_s(findData.cFileName, MAX_PATH, QDir::toNativeSeparators(item._file).toStdWString().c_str());
        findData.dwFileAttributes = FILE_ATTRIBUTE_UNPINNED;
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
            qCWarning(lcVfsWin) << "Error in CFPConvertToPlaceHolder!";
        }
    }
    else {
        // Download of temporary file finished
        if (CFPUpdateFetchStatus(
                    _setupParams.account.get()->driveId().toStdWString().c_str(),
                    QDir::toNativeSeparators(replacesFile).toStdWString().c_str(),
                    QDir::toNativeSeparators(filePath).toStdWString().c_str(),
                    item._size) != S_OK) {
            qCWarning(lcVfsWin) << "Error in CFPUpdateFetchStatus!";
            return false;
        }

        // Update file type in DB
        QString relativePath = filePath.midRef(_setupParams.filesystemPath.size()).toUtf8();
        OCC::SyncJournalFileRecord record;
        if (_setupParams.journal->getFileRecord(relativePath, &record) && record.isValid()) {
            record._type = ItemTypeFile;
            _setupParams.journal->setFileRecord(record);
        }
        // Update pin state in DB
        setPinStateInDb(relativePath, OCC::PinState::AlwaysLocal);
    }

    return true;
}

bool VfsWin::isDehydratedPlaceholder(const QString &fileRelativePath)
{
    bool isDehydrated;
    if (CFPGetPlaceHolderStatus(
                QDir::toNativeSeparators(_setupParams.filesystemPath).toStdWString().c_str(),
                fileRelativePath.toStdWString().c_str(),
                nullptr,
                &isDehydrated,
                nullptr) != S_OK) {
        qCWarning(lcVfsWin) << "Error in CFPGetPlaceHolderStatus!";
        return false;
    }

    return isDehydrated;
}

bool VfsWin::statTypeVirtualFile(csync_file_stat_t *stat, void *stat_data, const QString &fileDirectory)
{
    Q_UNUSED(stat_data)

    bool isPlaceholder;
    bool isDehydrated;
    bool isDirectory;
    if (CFPGetPlaceHolderStatus(
                QDir::toNativeSeparators(fileDirectory).toStdWString().c_str(),
                QDir::toNativeSeparators(stat->path).toStdWString().c_str(),
                &isPlaceholder,
                &isDehydrated,
                &isDirectory) != S_OK) {
        qCWarning(lcVfsWin) << "Error in CFPGetPlaceHolderStatus!";
        return false;
    }

    if (isPlaceholder) {
        if (isDehydrated) {
            stat->type = ItemTypeVirtualFile;
            return true;
        }

        if (isDirectory) {
            stat->type = ItemTypeDirectory;
            return true;
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
    qCDebug(lcVfsWin) << "Status of " << filePath << " : " << status.toSocketAPIString();

    if (!OCC::FileSystem::fileExists(filePath)) {
        return;
    }

    if (QDir(filePath).exists()) {
        return;
    }

    QString relativePath = filePath.midRef(_setupParams.filesystemPath.size()).toUtf8();
    if (status.tag() == OCC::SyncFileStatus::StatusExcluded ||
            status.tag() == OCC::SyncFileStatus::StatusUpToDate) {
        return;
    }
    else if (status.tag() == OCC::SyncFileStatus::StatusSync) {
        auto localPinState = pinState(relativePath);
        auto dbPinState = pinStateInDb(relativePath);
        bool isDehydrated = isDehydratedPlaceholder(relativePath);
        if (*localPinState == OCC::PinState::OnlineOnly && !isDehydrated) {
            // Dehydrate file
            if (CFPDehydratePlaceHolder(
                    QDir::toNativeSeparators(_setupParams.filesystemPath).toStdWString().c_str(),
                    QDir::toNativeSeparators(relativePath).toStdWString().c_str())) {
                qCWarning(lcVfsWin) << "Error in CFPDehydratePlaceHolder!";
                return;
            }
            // Update file type in DB
            OCC::SyncJournalFileRecord record;
            if (_setupParams.journal->getFileRecord(relativePath, &record) && record.isValid()) {
                record._type = ItemTypeVirtualFile;
                _setupParams.journal->setFileRecord(record);
            }
            // Update pin state in DB
            setPinStateInDb(relativePath, OCC::PinState::OnlineOnly);
        }
        else if (*localPinState == OCC::PinState::AlwaysLocal && isDehydrated) {
            // Hydrate file
            auto hydrateFct = [=]() {
                if (CFPHydratePlaceHolder(
                        QDir::toNativeSeparators(_setupParams.filesystemPath).toStdWString().c_str(),
                        QDir::toNativeSeparators(relativePath).toStdWString().c_str())) {
                    qCWarning(lcVfsWin) << "Error in CFPDehydratePlaceHolder!";
                    return;
                }
                // Update file type in DB
                OCC::SyncJournalFileRecord record;
                if (_setupParams.journal->getFileRecord(relativePath, &record) && record.isValid()) {
                    record._type = ItemTypeFile;
                    _setupParams.journal->setFileRecord(record);
                }
                // Update pin state in DB
                setPinStateInDb(relativePath, OCC::PinState::AlwaysLocal);
            };
            std::thread hydrateTask(hydrateFct);
            hydrateTask.detach();
        }
        else if (*dbPinState != *localPinState) {
            qCDebug(lcVfsWin) << "Update DB type and pin state inconsistency";

            // Update file type in DB
            OCC::SyncJournalFileRecord record;
            if (_setupParams.journal->getFileRecord(relativePath, &record) && record.isValid()) {
                record._type = isDehydrated ? ItemTypeVirtualFile : ItemTypeFile;
                _setupParams.journal->setFileRecord(record);
            }
            // Update pin state in DB
            setPinStateInDb(relativePath, isDehydrated ? OCC::PinState::OnlineOnly : OCC::PinState::AlwaysLocal);
        }
    }
    else if (status.tag() == OCC::SyncFileStatus::StatusWarning ||
            status.tag() == OCC::SyncFileStatus::StatusError) {
        if (CFPCancelFetch(
                    _setupParams.account.get()->driveId().toStdWString().c_str(),
                    QDir::toNativeSeparators(filePath).toStdWString().c_str()) != S_OK) {
            qCWarning(lcVfsWin) << "Error in CFPCancelFetch!";
            return;
        }
    }
}

} // namespace OCC
