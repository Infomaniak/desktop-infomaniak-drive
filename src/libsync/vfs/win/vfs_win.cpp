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

#include <QFile>
#include <QDir>

namespace KDC {

Q_LOGGING_CATEGORY(lcVfsWin, "vfswin", QtInfoMsg)

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

    CFPInitCloudFileProvider(debugCbk, QString(APPLICATION_SHORTNAME).toStdWString().c_str());

    auto watchFct = [params]() {
        if (CFPStartCloudFileProvider(
                    params.account.get()->driveId().toStdWString().c_str(),
                    params.providerName.toStdWString().c_str(),
                    params.account.get()->id().toStdWString().c_str(),
                    QDir::toNativeSeparators(params.filesystemPath).toStdWString().c_str()) != S_OK) {
            qCDebug(lcVfsWin) << "CFPStartCloudFileProvider failed!";
        }
    };

    std::thread watchThread(watchFct);
    watchThread.detach();
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
    findData.nFileSizeHigh = HIWORD(item._size);
    findData.nFileSizeLow = LOWORD(item._size);
    OCC::Utility::UnixTimeToFiletime(item._modtime, &findData.ftLastWriteTime);
    findData.ftCreationTime = findData.ftLastWriteTime;
    findData.ftLastAccessTime = findData.ftLastWriteTime;
    if (item._type == ItemTypeDirectory) {
        findData.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
    }
    else if (item._type == ItemTypeSoftLink) {
        findData.dwFileAttributes = FILE_ATTRIBUTE_REPARSE_POINT;
    }

    if (CFPCreatePlaceHolder(
                QString(item._fileId).toStdWString().c_str(),
                _setupParams.remotePath.toStdWString().c_str(),
                _setupParams.filesystemPath.toStdWString().c_str(),
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

void VfsWin::convertToPlaceholder(const QString &, const OCC::SyncFileItem &, const QString &)
{
    // Nothing necessary
}

bool VfsWin::isDehydratedPlaceholder(const QString &filePath)
{
    return CFPIsDehydratedPlaceHolder(filePath.toStdWString().c_str());
}

bool VfsWin::statTypeVirtualFile(csync_file_stat_t *stat, void *)
{
    if (CFPIsDehydratedPlaceHolder(QString(stat->path).toStdWString().c_str())) {
        stat->type = ItemTypeVirtualFile;
        return true;
    }
    return false;
}

OCC::Vfs::AvailabilityResult VfsWin::availability(const QString &folderPath)
{
    return availabilityInDb(folderPath);
}

} // namespace OCC
