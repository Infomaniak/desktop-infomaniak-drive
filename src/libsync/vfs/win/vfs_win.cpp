/*
 * Copyright (C) by Christian Kamm <mail@ckamm.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#include "vfs_win.h"
#include "syncfileitem.h"
#include "filesystem.h"
#include "common/syncjournaldb.h"
#include "CloudFileProvider.h"

#include <windows.h>
#include <iostream>
#include <thread>

#include <QFile>

namespace OCC {

HINSTANCE hCloudFileProviderDll;

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

Vfs::Mode VfsWin::mode() const
{
    return WindowsCfApi;
}

QString VfsWin::fileSuffix() const
{
    return QString();
}

void VfsWin::startImpl(const VfsSetupParams &params)
{
    Q_UNUSED(params)

    qCDebug(lcVfsWin) << "Begin";

    auto watchFct = [=]() {
        const wchar_t serverFolder[] = L"C:\\temp\\sync_server";
        const wchar_t clientFolder[] = L"C:\\temp\\sync_client";
        qCDebug(lcVfsWin) << "StartCloudFileProvider() returned "
                  << StartCloudFileProvider(serverFolder, clientFolder, debugCbk);
    };

    std::thread watchThread(watchFct);
    watchThread.detach();
}

void VfsWin::stop()
{
    qCDebug(lcVfsWin) << "StopCloudFileProvider() returned " << StopCloudFileProvider();

    FreeLibrary(hCloudFileProviderDll);
}

void VfsWin::unregisterFolder()
{
}

bool VfsWin::isHydrating() const
{
    return false;
}

bool VfsWin::updateMetadata(const QString &filePath, time_t modtime, qint64, const QByteArray &, QString *)
{
    FileSystem::setModTime(filePath, modtime);
    return true;
}

void VfsWin::createPlaceholder(const SyncFileItem &item)
{
    // The concrete shape of the placeholder is also used in isDehydratedPlaceholder() below
    QString fn = _setupParams.filesystemPath + item._file;
    if (!fn.endsWith(fileSuffix())) {
        ASSERT_2(false, "vfs file isn't ending with suffix");
        return;
    }

    QFile file(fn);
    file.open(QFile::ReadWrite | QFile::Truncate);
    file.write(" ");
    file.close();
    FileSystem::setModTime(fn, item._modtime);
}

void VfsWin::dehydratePlaceholder(const SyncFileItem &item)
{
    QFile::remove(_setupParams.filesystemPath + item._file);
    SyncFileItem virtualItem(item);
    virtualItem._file = item._renameTarget;
    createPlaceholder(virtualItem);

    // Move the item's pin state
    auto pin = _setupParams.journal->internalPinStates().rawForPath(item._file.toUtf8());
    if (pin && *pin != PinState::Inherited) {
        setPinState(item._renameTarget, *pin);
        setPinState(item._file, PinState::Inherited);
    }

    // Ensure the pin state isn't contradictory
    pin = pinState(item._renameTarget);
    if (pin && *pin == PinState::AlwaysLocal)
        setPinState(item._renameTarget, PinState::Unspecified);
}

void VfsWin::convertToPlaceholder(const QString &, const SyncFileItem &, const QString &)
{
    // Nothing necessary
}

bool VfsWin::isDehydratedPlaceholder(const QString &filePath)
{
    if (!filePath.endsWith(fileSuffix()))
        return false;
    QFileInfo fi(filePath);
    return fi.exists() && fi.size() == 1;
}

bool VfsWin::statTypeVirtualFile(csync_file_stat_t *stat, void *)
{
    if (stat->path.endsWith(fileSuffix().toUtf8())) {
        stat->type = ItemTypeVirtualFile;
        return true;
    }
    return false;
}

Vfs::AvailabilityResult VfsWin::availability(const QString &folderPath)
{
    return availabilityInDb(folderPath);
}

} // namespace OCC
