/*
 * Copyright (C) by Dominik Schmidt <dschmidt@owncloud.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "vfs.h"
#include "plugin.h"
#include "version.h"
#include "syncjournaldb.h"

#include <QPluginLoader>
#include <QLoggingCategory>
#include <QOperatingSystemVersion>

#define MIN_WINDOWS10_MICROVERSION_FOR_CFAPI 16299 // Windows 10 version 1709

using namespace OCC;

Vfs::Vfs(QObject* parent)
    : QObject(parent)
{
}

Vfs::~Vfs() = default;

QString Vfs::modeToString(Mode mode)
{
    // Note: Strings are used for config and must be stable
    switch (mode) {
    case Off:
        return QStringLiteral("off");
    case WithSuffix:
        return QStringLiteral("suffix");
    case WindowsCfApi:
        return QStringLiteral("wincfapi");
    }
    return QStringLiteral("off");
}

Optional<Vfs::Mode> Vfs::modeFromString(const QString &str)
{
    // Note: Strings are used for config and must be stable
    if (str == "off") {
        return Off;
    } else if (str == "suffix") {
        return WithSuffix;
    } else if (str == "wincfapi") {
        return WindowsCfApi;
    }
    return {};
}

void Vfs::start(const VfsSetupParams &params)
{
    _setupParams = params;
    startImpl(params);
}

bool Vfs::setPinStateInDb(const QString &fileRelativePath, PinState state)
{
    auto path = fileRelativePath.toUtf8();
    _setupParams.journal->internalPinStates().wipeForPathAndBelow(path);
    if (state != PinState::Inherited)
        _setupParams.journal->internalPinStates().setForPath(path, state);
    return true;
}

Optional<PinState> Vfs::pinStateInDb(const QString &fileRelativePath)
{
    auto pin = _setupParams.journal->internalPinStates().effectiveForPath(fileRelativePath.toUtf8());
    return pin;
}

Vfs::AvailabilityResult Vfs::availabilityInDb(const QString &fileRelativePath)
{
    auto path = fileRelativePath.toUtf8();
    auto pin = _setupParams.journal->internalPinStates().effectiveForPathRecursive(path);
    // not being able to retrieve the pin state isn't too bad
    auto hydrationStatus = _setupParams.journal->hasHydratedOrDehydratedFiles(path);
    if (!hydrationStatus)
        return AvailabilityError::DbError;

    if (hydrationStatus->hasDehydrated) {
        if (hydrationStatus->hasHydrated)
            return VfsItemAvailability::Mixed;
        if (pin && *pin == PinState::OnlineOnly)
            return VfsItemAvailability::OnlineOnly;
        else
            return VfsItemAvailability::AllDehydrated;
    } else if (hydrationStatus->hasHydrated) {
        if (pin && *pin == PinState::AlwaysLocal)
            return VfsItemAvailability::AlwaysLocal;
        else
            return VfsItemAvailability::AllHydrated;
    }
    return AvailabilityError::NoSuchItem;
}

VfsOff::VfsOff(QObject *parent)
    : Vfs(parent)
{
}

VfsOff::~VfsOff() = default;

static QString modeToPluginName(Vfs::Mode mode)
{
    if (mode == Vfs::WithSuffix)
        return "suffix";
    if (mode == Vfs::WindowsCfApi)
        return "win";
    return QString();
}

Q_LOGGING_CATEGORY(lcPlugin, "sync.plugins", QtInfoMsg)

bool OCC::isVfsPluginAvailable(Vfs::Mode mode)
{
    if (mode == Vfs::Off)
        return true;

    if (mode == Vfs::WindowsCfApi) {
        if (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows10
                && QOperatingSystemVersion::current().microVersion() >= MIN_WINDOWS10_MICROVERSION_FOR_CFAPI) {
            return true;
        }
        else {
            return false;
        }
    }

    auto name = modeToPluginName(mode);
    if (name.isEmpty())
        return false;
    auto pluginPath = pluginFileName("vfs", name);
    QPluginLoader loader(pluginPath);

    auto basemeta = loader.metaData();
    if (basemeta.isEmpty() || !basemeta.contains("IID")) {
        qCDebug(lcPlugin) << "Plugin doesn't exist" << pluginPath;
        return false;
    }
    if (basemeta["IID"].toString() != "org.kdrive.PluginFactory") {
        qCWarning(lcPlugin) << "Plugin has wrong IID" << pluginPath << basemeta["IID"];
        return false;
    }

    auto metadata = basemeta["MetaData"].toObject();
    if (metadata["type"].toString() != "vfs") {
        qCWarning(lcPlugin) << "Plugin has wrong type" << pluginPath << metadata["type"];
        return false;
    }
    if (metadata["version"].toString() != MIRALL_VERSION_STRING) {
        qCWarning(lcPlugin) << "Plugin has wrong version" << pluginPath << metadata["version"];
        return false;
    }

    // Attempting to load the plugin is essential as it could have dependencies that
    // can't be resolved and thus not be available after all.
    if (!loader.load()) {
        qCWarning(lcPlugin) << "Plugin failed to load:" << loader.errorString();
        return false;
    }

    return true;
}

Vfs::Mode OCC::bestAvailableVfsMode(bool showExperimentalOptions)
{
    if (isVfsPluginAvailable(Vfs::WindowsCfApi)) {
        return Vfs::WindowsCfApi;
    } else if (showExperimentalOptions && isVfsPluginAvailable(Vfs::WithSuffix)) {
        return Vfs::WithSuffix;
    }
    return Vfs::Off;
}

std::unique_ptr<Vfs> OCC::createVfsFromPlugin(Vfs::Mode mode)
{
    if (mode == Vfs::Off)
        return std::unique_ptr<Vfs>(new VfsOff);

    auto name = modeToPluginName(mode);
    if (name.isEmpty())
        return nullptr;
    auto pluginPath = pluginFileName("vfs", name);

    if (!isVfsPluginAvailable(mode)) {
        qCWarning(lcPlugin) << "Could not load plugin: not existant or bad metadata" << pluginPath;
        return nullptr;
    }

    QPluginLoader loader(pluginPath);
    auto plugin = loader.instance();
    if (!plugin) {
        qCWarning(lcPlugin) << "Could not load plugin" << pluginPath << loader.errorString();
        return nullptr;
    }

    auto factory = qobject_cast<PluginFactory *>(plugin);
    if (!factory) {
        qCWarning(lcPlugin) << "Plugin" << pluginPath << "does not implement PluginFactory";
        return nullptr;
    }

    auto vfs = std::unique_ptr<Vfs>(qobject_cast<Vfs *>(factory->create(nullptr)));
    if (!vfs) {
        qCWarning(lcPlugin) << "Plugin" << pluginPath << "does not create a Vfs instance";
        return nullptr;
    }

    qCInfo(lcPlugin) << "Created VFS instance from plugin" << pluginPath;
    return vfs;
}
