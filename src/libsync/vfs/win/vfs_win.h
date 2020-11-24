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

#pragma once

#include "common/vfs.h"
#include "common/plugin.h"

#include <windows.h>

#include <QObject>
#include <QScopedPointer>

namespace KDC {

class VfsWin : public OCC::Vfs
{
    Q_OBJECT

public:
    explicit VfsWin(QObject *parent = nullptr);
    ~VfsWin();

    Mode mode() const override;
    QString fileSuffix() const override;

    void stop() override;
    void unregisterFolder() override;

    bool socketApiPinStateActionsShown() const override { return false; }
    bool isHydrating() const override;

    bool updateMetadata(const QString &filePath, time_t modtime, qint64 size, const QByteArray &fileId, QString *error) override;

    void createPlaceholder(const OCC::SyncFileItem &item) override;
    void dehydratePlaceholder(const OCC::SyncFileItem &item) override;
    bool convertToPlaceholder(const QString &filePath, const OCC::SyncFileItem &item, const QString &replacesFile) override;

    bool needsMetadataUpdate(const OCC::SyncFileItem &) override { return true; }
    bool isDehydratedPlaceholder(const QString &fileRelativePath) override;
    bool statTypeVirtualFile(csync_file_stat_t *stat, void *stat_data, const QString &fileDirectory) override;

    bool setPinState(const QString &fileRelativePath, OCC::PinState state) override;
    OCC::Optional<OCC::PinState> pinState(const QString &fileRelativePath) override;
    AvailabilityResult availability(const QString &fileRelativePath) override;

public slots:
    void fileStatusChanged(const QString &filePath, OCC::SyncFileStatus status) override;

protected:
    void startImpl(const OCC::VfsSetupParams &params, QString &namespaceCLSID = QString()) override;

private:
    void dehydrateFile(const QString &filePath);
    void hydrateFile(const QString &filePath);
    DWORD getPlaceholderAttributes(const QString &filePath);
    void setPlaceholderStatus(const QString &filePath, bool inSync);
};

class WinVfsPluginFactory : public QObject, public OCC::DefaultPluginFactory<VfsWin>
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kdrive.PluginFactory" FILE "vfspluginmetadata.json")
    Q_INTERFACES(OCC::PluginFactory)
};

} // namespace KDC
