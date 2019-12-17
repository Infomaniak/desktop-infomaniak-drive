/*
 * Copyright (C) by Klaas Freitag <freitag@owncloud.com>
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

#include "infomaniaktheme.h"

#include <QString>
#include <QVariant>
#ifndef TOKEN_AUTH_ONLY
#include <QPixmap>
#include <QIcon>
#endif
#include <QCoreApplication>
#include <QSslSocket>

#include "config.h"
#include "common/utility.h"
#include "common/vfs.h"
#include "version.h"

namespace OCC {

InfomaniakTheme::InfomaniakTheme()
    : Theme()
{
}

bool InfomaniakTheme::wizardHideExternalStorageConfirmationCheckbox() const
{
    return true;
}

QString InfomaniakTheme::systrayIconFlavor(bool mono, bool sysTrayMenuVisible) const
{
    Q_UNUSED(sysTrayMenuVisible)
    QString flavor;
    if (mono) {
        flavor = Utility::hasDarkSystray() ? QLatin1String("white") : QLatin1String("black");

#ifdef Q_OS_MAC
        if (sysTrayMenuVisible) {
            flavor = QLatin1String("white");
        }
#endif
    } else {
        flavor = QLatin1String("systray_colored");
    }
    return flavor;
}

QString InfomaniakTheme::gitSHA1() const
{
    QString devString;
#ifdef GIT_SHA1
    const QString githubPrefix(QLatin1String(
        "https://github.com/infomaniak/desktop-infomaniak-drive/commit/"));
    const QString gitSha1(QLatin1String(GIT_SHA1));
    devString = QCoreApplication::translate("nextcloudTheme::about()",
        "<p><small>Built from Git revision <a style=\"color: #489EF3\" href=\"%1\">%2</a>"
        " on %3, %4 using Qt %5, %6</small></p>")
                    .arg(githubPrefix + gitSha1)
                    .arg(gitSha1.left(6))
                    .arg(__DATE__)
                    .arg(__TIME__)
                    .arg(qVersion())
                    .arg(QSslSocket::sslLibraryVersionString());
#endif
    return devString;
}

QString InfomaniakTheme::helpUrl() const
{
#ifdef APPLICATION_HELP_URL
    return QString::fromLatin1(APPLICATION_HELP_URL);
#else
    return QString::fromLatin1("https://faq.infomaniak.com/2353");
#endif
}

QString InfomaniakTheme::conflictHelpUrl() const
{
    return QString();
    // return QString::fromLatin1("https://faq.infomaniak.com/2353");
}

QString InfomaniakTheme::overrideServerUrl() const
{
#ifdef APPLICATION_SERVER_URL
    return QString::fromLatin1(APPLICATION_SERVER_URL);
#else
    return QString();
#endif
}

QString InfomaniakTheme::about() const
{
    QString vendor = APPLICATION_VENDOR;
    // Ideally, the vendor should be "ownCloud GmbH", but it cannot be changed without
    // changing the location of the settings and other registery keys.
    if (vendor == "ownCloud") vendor = QLatin1String("ownCloud GmbH");

    QString devString;
    devString = tr("<p>Version %2. For more information visit <a style=\"color: #489EF3\" href=\"%3\">https://%4</a></p>")
                    .arg(Utility::escape(MIRALL_VERSION_STRING),
                        Utility::escape("https://" MIRALL_STRINGIFY(APPLICATION_DOMAIN)),
                        Utility::escape(MIRALL_STRINGIFY(APPLICATION_DOMAIN)));
    devString += tr("<p>Copyright Infomaniak Network SA, ownCloud GmbH and Nextcloud GmbH</p>");
    devString += tr("<p>Distributed by %1 and licensed under the GNU General Public License (GPL) Version 2.0.<br/>"
                    "%2 and the %2 logo are registered trademarks of %1.</p>")
               .arg(Utility::escape(vendor), Utility::escape(APPLICATION_NAME));

    devString += gitSHA1();
    devString += QString("<p><small>Using virtual files plugin: %1</small></p>")
        .arg(Vfs::modeToString(bestAvailableVfsMode()));

    return devString;
}

QString InfomaniakTheme::updateCheckUrl() const
{
    return APPLICATION_UPDATE_URL;
}

bool InfomaniakTheme::userGroupSharing() const
{
    return false;
}

}
