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

void notifyCbk(NotificationType type, const wchar_t *filePath) {
    QString filePathStr(QDir::toNativeSeparators(QString::fromStdWString(filePath)));
    if (type == NotificationType::NOTIFICATION_TYPE_FETCH_DATA) {
        s_fetchMap[filePathStr] = OCC::SyncFileStatus::SyncFileStatusTag::StatusSync;
    }
    else if (type == NotificationType::NOTIFICATION_TYPE_CANCEL_FETCH_DATA) {
        if (s_fetchMap.find(filePathStr) != s_fetchMap.end()) {
            s_fetchMap.erase(filePathStr);




        }
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

    CFPInitCloudFileProvider(debugCbk, notifyCbk, QString(APPLICATION_SHORTNAME).toStdWString().c_str());

    auto callFct = [params]() {
        if (CFPStartCloudFileProvider(
                    params.account.get()->driveId().toStdWString().c_str(),
                    params.providerName.toStdWString().c_str(),
                    params.account.get()->davUser().toStdWString().c_str(),
                    QDir::toNativeSeparators(params.filesystemPath).toStdWString().c_str()) != S_OK) {
            qCDebug(lcVfsWin) << "CFPStartCloudFileProvider failed!";
        }
    };

    std::thread callThread(callFct);
    callThread.detach();
}

void VfsWin::updateFileStatus(const QString &fileName, const QString &fromFileName,
                              OCC::SyncFileStatus status, qint64 completed)
{
    QString filePathStr(QDir::toNativeSeparators(fileName));
    if (s_fetchMap.find(filePathStr) != s_fetchMap.end()) {
        qCDebug(lcVfsWin) << "Status of " << filePathStr << " : " << status.toSocketAPIString();

        if (status.tag() != OCC::SyncFileStatus::StatusSync) {
            // End of fetch
            s_fetchMap.erase(filePathStr);
        }

        QString fromFilePathStr(QDir::toNativeSeparators(fromFileName));
        if (CFPUpdateFetchStatus(
                    _setupParams.account.get()->driveId().toStdWString().c_str(),
                    filePathStr.toStdWString().c_str(),
                    fromFilePathStr.toStdWString().c_str(),
                    (FetchStatus) status.tag(),
                    completed) != S_OK) {
            qCDebug(lcVfsWin) << "CFPUpdateFetchStatus failed!";
        }
    }
}

void VfsWin::stop()
{
}

void VfsWin::unregisterFolder()
{
    if (CFPStopCloudFileProvider(_setupParams.account.get()->driveId().toStdWString().c_str()) != S_OK) {
        qCDebug(lcVfsWin) << "CFPStopCloudFileProvider failed!";
    }
}

bool VfsWin::isHydrating() const
{
    return CFPIsHydrating(_setupParams.account.get()->driveId().toStdWString().c_str());
}

bool VfsWin::updateMetadata(const QString &filePath, time_t modtime, qint64, const QByteArray &, QString *)
{
    OCC::FileSystem::setModTime(filePath, modtime);
    return true;
}

void VfsWin::createPlaceholder(const OCC::SyncFileItem &item)
{
    WIN32_FIND_DATA findData;
    wcscpy_s(findData.cFileName, MAX_PATH, item._file.toStdWString().c_str());
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
                QDir::toNativeSeparators(_setupParams.remotePath).toStdWString().c_str(),
                QDir::toNativeSeparators(_setupParams.filesystemPath).toStdWString().c_str(),
                &findData) != S_OK) {
        qCDebug(lcVfsWin) << "CFPCreatePlaceHolder failed!";
    }
}

void VfsWin::dehydratePlaceholder(const OCC::SyncFileItem &item)
{
    QFile::remove(_setupParams.filesystemPath + item._file);
    OCC::SyncFileItem virtualItem(item);
    virtualItem._file = item._renameTarget;
    createPlaceholder(virtualItem);

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
}

void VfsWin::convertToPlaceholder(const QString &fileName, const OCC::SyncFileItem &item, const QString &replacesFile)
{
    Q_UNUSED(fileName)

    createPlaceholder(item);

    if (!replacesFile.isEmpty()) {
        // Download of temporary file finished
        updateFileStatus(replacesFile, fileName, OCC::SyncFileStatus::StatusSync, item._size);
    }
}

bool VfsWin::isDehydratedPlaceholder(const QString &fileRelativePath)
{
    DWORD dwAttrs;

    QString filePath(_setupParams.filesystemPath + fileRelativePath);
    dwAttrs = GetFileAttributesW(filePath.toStdWString().c_str());

    if (dwAttrs != INVALID_FILE_ATTRIBUTES) {
        if (!(dwAttrs & FILE_ATTRIBUTE_DIRECTORY) && (dwAttrs & FILE_ATTRIBUTE_PINNED))
        {
            return true;
        }
    }
    return false;
}

bool VfsWin::statTypeVirtualFile(csync_file_stat_t *stat, void *)
{
    if (isDehydratedPlaceholder(QDir::toNativeSeparators(stat->path))) {
        stat->type = ItemTypeVirtualFile;
        return true;
    }
    return false;
}

bool VfsWin::setPinState(const QString &fileRelativePath, OCC::PinState state)
{
    DWORD dwAttrs;

    QString filePath(_setupParams.filesystemPath + fileRelativePath);
    dwAttrs = GetFileAttributesW(filePath.toStdWString().c_str());

    if (dwAttrs != INVALID_FILE_ATTRIBUTES) {
        switch (state) {
        case OCC::PinState::AlwaysLocal:
            dwAttrs |= FILE_ATTRIBUTE_PINNED;
            dwAttrs &= ~FILE_ATTRIBUTE_UNPINNED;
            SetFileAttributesW(filePath.toStdWString().c_str(), dwAttrs);
            break;
        case OCC::PinState::OnlineOnly:
            dwAttrs |= FILE_ATTRIBUTE_UNPINNED;
            dwAttrs &= ~FILE_ATTRIBUTE_PINNED;
            SetFileAttributesW(filePath.toStdWString().c_str(), dwAttrs);
            break;
        case OCC::PinState::Inherited:
        case OCC::PinState::Unspecified:
            dwAttrs &= ~FILE_ATTRIBUTE_PINNED;
            dwAttrs &= ~FILE_ATTRIBUTE_UNPINNED;
            SetFileAttributesW(filePath.toStdWString().c_str(), dwAttrs);
            break;
        }

        return setPinStateInDb(fileRelativePath, state);;
    }
    return false;
}

OCC::Optional<OCC::PinState> VfsWin::pinState(const QString &fileRelativePath)
{
    DWORD dwAttrs;

    QString filePath(_setupParams.filesystemPath + fileRelativePath);
    dwAttrs = GetFileAttributesW(filePath.toStdWString().c_str());

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

void VfsWin::fileStatusChanged(const QString &fileName, OCC::SyncFileStatus status)
{
    updateFileStatus(fileName, nullptr, status, 0);
}

} // namespace OCC
