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
    void convertToPlaceholder(const QString &fileName, const OCC::SyncFileItem &item, const QString &replacesFile) override;

    bool needsMetadataUpdate(const OCC::SyncFileItem &) override { return true; }
    bool isDehydratedPlaceholder(const QString &fileRelativePath) override;
    bool statTypeVirtualFile(csync_file_stat_t *stat, void *stat_data) override;

    bool setPinState(const QString &fileRelativePath, OCC::PinState state) override;
    OCC::Optional<OCC::PinState> pinState(const QString &fileRelativePath) override;
    AvailabilityResult availability(const QString &fileRelativePath) override;

public slots:
    void fileStatusChanged(const QString &fileName, OCC::SyncFileStatus status) override;

protected:
    void startImpl(const OCC::VfsSetupParams &params) override;
    void updateFileStatus(const QString &fileName, const QString &fromFileName,
                          OCC::SyncFileStatus status, qint64 completed);
};

class WinVfsPluginFactory : public QObject, public OCC::DefaultPluginFactory<VfsWin>
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kdrive.PluginFactory" FILE "vfspluginmetadata.json")
    Q_INTERFACES(OCC::PluginFactory)
};

} // namespace KDC
