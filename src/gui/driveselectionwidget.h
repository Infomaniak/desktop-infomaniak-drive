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
    Q_PROPERTY(QColor add_icon_color READ addIconColor WRITE setAddIconColor)

public:
    explicit DriveSelectionWidget(QWidget *parent = nullptr);
    QSize sizeHint() const override;

    void clear();
    void addOrUpdateDrive(QString id, const QString &name, const QColor &color, OCC::SyncResult::Status status);
    void removeDrive(QString id);
    void selectDrive(QString id);

    inline QSize driveIconSize() const { return _driveIconSize; }
    inline void setDriveIconSize(QSize size) {
        _driveIconSize = size;
        emit driveIconSizeChanged();
    }

    inline QSize downIconSize() const { return _downIconSize; }
    inline void setDownIconSize(QSize size) {
        _downIconSize = size;
        emit downIconSizeChanged();
    }

    inline QColor downIconColor() const { return _downIconColor; }
    inline void setDownIconColor(QColor color) {
        _downIconColor = color;
        emit downIconColorChanged();
    }

    inline QSize menuRightIconSize() const { return _menuRightIconSize; }
    inline void setMenuRightIconSize(QSize size) { _menuRightIconSize = size; }

    inline QColor addIconColor() const { return _addIconColor; }
    inline void setAddIconColor(QColor color) {
        _addIconColor = color;
        emit addIconColorChanged();
    }

signals:
    void driveIconSizeChanged();
    void downIconSizeChanged();
    void downIconColorChanged();
    void addIconColorChanged();
    void driveSelected(QString id);
    void addDrive();

private:
    struct DriveInfo {
        QString _name;
        QColor _color;
        OCC::SyncResult::Status _status;
    };

    QSize _driveIconSize;
    QSize _downIconSize;
    QColor _downIconColor;
    QSize _menuRightIconSize;
    QColor _addIconColor;
    std::map<QString, DriveInfo> _driveMap;
    QString _currentDriveId;
    QLabel *_driveIconLabel;
    QLabel *_driveTextLabel;
    QLabel *_downIconLabel;

    void setDriveIcon(const QColor &color);
    void setAddDriveIcon();
    void setDownIcon();

private slots:
    void onDriveIconSizeChanged();
    void onDownIconSizeChanged();
    void onDownIconColorChanged();
    void onAddIconColorChanged();
    void onClick(bool checked);
    void onSelectDriveActionTriggered(bool checked = false);
    void onAddDriveActionTriggered(bool checked = false);
};

}
