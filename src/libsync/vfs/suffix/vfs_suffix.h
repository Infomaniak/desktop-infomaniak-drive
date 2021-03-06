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

#include <QObject>
#include <QScopedPointer>

#include "common/vfs.h"
#include "common/plugin.h"

namespace OCC {

class VfsSuffix : public Vfs
{
    Q_OBJECT

public:
    explicit VfsSuffix(QObject *parent = nullptr);
    ~VfsSuffix();

    Mode mode() const override;
    QString fileSuffix() const override;

    void stop() override;
    void unregisterFolder() override;

    bool socketApiPinStateActionsShown() const override { return true; }
    bool isHydrating() const override;

    bool updateMetadata(const QString &filePath, time_t modtime, qint64 size, const QByteArray &fileId, QString *error) override;

    bool createPlaceholder(const SyncFileItem &item) override;
    void dehydratePlaceholder(const SyncFileItem &item) override;
    bool convertToPlaceholder(const QString &, const SyncFileItem &) override;
    bool updateFetchStatus(const QString &, const QString &, qint64, bool &) override;

    bool needsMetadataUpdate(const SyncFileItem &) override { return false; }
    bool isDehydratedPlaceholder(const QString &fileRelativePath) override;
    bool statTypeVirtualFile(csync_file_stat_t *stat, void *stat_data, const QString &) override;

    bool setPinState(const QString &fileRelativePath, PinState state) override
    { return setPinStateInDb(fileRelativePath, state); }
    Optional<PinState> pinState(const QString &fileRelativePath) override
    { return pinStateInDb(fileRelativePath); }
    AvailabilityResult availability(const QString &fileRelativePath) override;

public slots:
    void fileStatusChanged(const QString &, SyncFileStatus) override {}

protected:
    void startImpl(const VfsSetupParams &params, QString &namespaceCLSID) override;
};

class SuffixVfsPluginFactory : public QObject, public DefaultPluginFactory<VfsSuffix>
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kdrive.PluginFactory" FILE "vfspluginmetadata.json")
    Q_INTERFACES(OCC::PluginFactory)
};

} // namespace OCC
