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

#include "accountinfo.h"
#include "theme.h"

#include <map>

#include <QFont>
#include <QColor>
#include <QLabel>
#include <QList>
#include <QPushButton>
#include <QSize>
#include <QString>

namespace KDC {

class DriveSelectionWidget : public QPushButton
{
    Q_OBJECT

    Q_PROPERTY(QSize drive_icon_size READ driveIconSize WRITE setDriveIconSize)
    Q_PROPERTY(QSize down_icon_size READ downIconSize WRITE setDownIconSize)
    Q_PROPERTY(QColor down_icon_color READ downIconColor WRITE setDownIconColor)
    Q_PROPERTY(QSize menu_right_icon_size READ menuRightIconSize WRITE setMenuRightIconSize)

public:
    explicit DriveSelectionWidget(QWidget *parent = nullptr);
    QSize sizeHint() const override;

    void clear();
    void addOrUpdateDrive(QString id, const AccountInfo &accountInfo);
    void removeDrive(QString id);
    void selectDrive(QString id);

signals:
    void driveSelected(QString id);
    void addDrive();

private:
    QSize _driveIconSize;
    QSize _downIconSize;
    QColor _downIconColor;
    QSize _menuRightIconSize;
    std::map<QString, AccountInfo> _driveMap;
    QString _currentDriveId;
    QLabel *_driveIconLabel;
    QLabel *_driveTextLabel;
    QLabel *_downIconLabel;

    inline QSize driveIconSize() const { return _driveIconSize; }
    inline void setDriveIconSize(QSize size) {
        _driveIconSize = size;
        setDriveIcon();
    }

    inline QSize downIconSize() const { return _downIconSize; }
    inline void setDownIconSize(QSize size) {
        _downIconSize = size;
        setDownIcon();
    }

    inline QColor downIconColor() const { return _downIconColor; }
    inline void setDownIconColor(QColor color) {
        _downIconColor = color;
        setDownIcon();
    }

    inline QSize menuRightIconSize() const { return _menuRightIconSize; }
    inline void setMenuRightIconSize(QSize size) { _menuRightIconSize = size; }

    void setDriveIcon();
    void setDriveIcon(const QColor &color);
    void setDownIcon();

private slots:
    void onClick(bool checked);
    void onSelectDriveActionTriggered(bool checked = false);
    void onAddDriveActionTriggered(bool checked = false);
};

}
