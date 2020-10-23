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
#pragma once

#include "common/vfs.h"
#include "common/plugin.h"

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
    void convertToPlaceholder(const QString &filename, const OCC::SyncFileItem &item, const QString &) override;

    bool needsMetadataUpdate(const OCC::SyncFileItem &) override { return false; }
    bool isDehydratedPlaceholder(const QString &filePath) override;
    bool statTypeVirtualFile(csync_file_stat_t *stat, void *stat_data) override;

    bool setPinState(const QString &folderPath, OCC::PinState state) override
    { return setPinStateInDb(folderPath, state); }
    OCC::Optional<OCC::PinState> pinState(const QString &folderPath) override
    { return pinStateInDb(folderPath); }
    AvailabilityResult availability(const QString &folderPath) override;

public slots:
    void fileStatusChanged(const QString &, OCC::SyncFileStatus) override {}

protected:
    void startImpl(const OCC::VfsSetupParams &params) override;
};

class WinVfsPluginFactory : public QObject, public OCC::DefaultPluginFactory<VfsWin>
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kdrive.PluginFactory" FILE "vfspluginmetadata.json")
    Q_INTERFACES(OCC::PluginFactory)
};

} // namespace KDC
