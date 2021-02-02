/*
 * Copyright (C) by Jocelyn Turcotte <jturcotte@woboq.com>
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

#include "navigationpanehelper.h"
#include "accountmanager.h"
#include "configfile.h"
#include "folderman.h"

#include <QDir>
#include <QCoreApplication>

namespace OCC {

Q_LOGGING_CATEGORY(lcNavPane, "gui.navigationpane", QtInfoMsg)

NavigationPaneHelper::NavigationPaneHelper(FolderMan *folderMan)
    : _folderMan(folderMan)
{
    ConfigFile cfg;
    _showInExplorerNavigationPane = cfg.showInExplorerNavigationPane();

#ifdef Q_OS_WIN
    _updateCloudStorageRegistryTimer.setSingleShot(true);
    connect(&_updateCloudStorageRegistryTimer, &QTimer::timeout, this, &NavigationPaneHelper::updateCloudStorageRegistry);
#endif
}

#ifdef Q_OS_WIN
void NavigationPaneHelper::setShowInExplorerNavigationPane(bool show)
{
    _showInExplorerNavigationPane = show;

    // Fix folders without CLSID (to remove later)
    foreach (Folder *folder, _folderMan->map()) {
        if (folder->vfs().mode() != Vfs::Mode::WindowsCfApi) {
            if (folder->navigationPaneClsid() == QUuid()) {
                folder->setNavigationPaneClsid(show ? QUuid::createUuid() : QUuid());
            }
        }
    }
    scheduleUpdateCloudStorageRegistry();

    // Set pin state
    foreach (Folder *folder, _folderMan->map()) {
        Utility::setFolderPinState(folder->navigationPaneClsid(), show);
    }
}

void NavigationPaneHelper::scheduleUpdateCloudStorageRegistry()
{
    // Schedule the update to happen a bit later to avoid doing the update multiple times in a row.
    if (!_updateCloudStorageRegistryTimer.isActive())
        _updateCloudStorageRegistryTimer.start(500);
}

void NavigationPaneHelper::updateCloudStorageRegistry()
{
    // Start by looking at every registered namespace extension for the sidebar, and look for an "ApplicationName" value
    // that matches ours when we saved.
    QVector<QUuid> entriesToRemove;
    Utility::registryWalkSubKeys(
        HKEY_CURRENT_USER,
        QStringLiteral("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\NameSpace"),
        [&entriesToRemove](HKEY key, const QString &subKey) {
            QVariant appName = Utility::registryGetKeyValue(key, subKey, QStringLiteral("ApplicationName"));
            if (appName.toString() == QLatin1String(APPLICATION_NAME)) {
                QUuid clsid{ subKey };
                Q_ASSERT(!clsid.isNull());
                entriesToRemove.append(clsid);
            }
        });

    // Then remove anything
    foreach (auto &clsid, entriesToRemove) {
        Utility::removeLegacySyncRootKeys(clsid);
    }

    // Then re-save every folder that has a valid navigationPaneClsid to the registry
    bool show = OCC::FolderMan::instance()->navigationPaneHelper().showInExplorerNavigationPane();
    foreach (Folder *folder, _folderMan->map()) {
        if (folder->vfs().mode() != Vfs::Mode::WindowsCfApi) {
            if (folder->navigationPaneClsid() == QUuid()) {
                folder->setNavigationPaneClsid(QUuid::createUuid());
            }
            Utility::addLegacySyncRootKeys(folder->navigationPaneClsid(), folder->path(), folder->cleanPath(), show);
        }
    }
}
#endif

} // namespace OCC
