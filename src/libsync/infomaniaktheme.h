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

#ifndef INFOMANIAK_THEME_H
#define INFOMANIAK_THEME_H

#include "theme.h"

namespace OCC {

/**
 * @brief The NextcloudTheme class
 * @ingroup libsync
 */
class InfomaniakTheme : public Theme
{
    Q_OBJECT
public:
    InfomaniakTheme();
    bool wizardHideExternalStorageConfirmationCheckbox() const override;
    QString systrayIconFlavor(bool mono, bool sysTrayMenuVisible = false) const override;
    QString gitSHA1() const override;
    QString helpUrl() const override;
    QString conflictHelpUrl() const override;
    QString overrideServerUrl() const override;
    QString about() const override;
    QString updateCheckUrl() const override;
    bool userGroupSharing() const override;
    bool deltaSynchronizationAvailable() const override;
};
}
#endif // INFOMANIAK_THEME_H
