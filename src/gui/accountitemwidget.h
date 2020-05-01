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

#include "accountitem.h"
#include "customtoolbutton.h"

#include <QColor>
#include <QLabel>
#include <QPaintEvent>
#include <QSize>
#include <QWidget>

namespace KDC {

class AccountItemWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QSize account_icon_size READ accountIconSize WRITE setAccountIconSize)
    Q_PROPERTY(QColor account_icon_color READ accountIconColor WRITE setAccountIconColor)
    Q_PROPERTY(QSize drive_icon_size READ driveIconSize WRITE setDriveIconSize)
    Q_PROPERTY(QSize status_icon_size READ statusIconSize WRITE setStatusIconSize)
    Q_PROPERTY(QColor background_color READ backgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(QColor background_color_selection READ backgroundColorSelection WRITE setBackgroundColorSelection)

public:
    explicit AccountItemWidget(const QString &accountId, QWidget *parent = nullptr);
    void updateItem(const AccountInfo &accountInfo);

    inline QSize accountIconSize() const { return _accountIconSize; }
    inline void setAccountIconSize(const QSize &size) {
        _accountIconSize = size;
        emit accountIconSizeChanged();
    }

    inline QColor accountIconColor() const { return _accountIconColor; }
    inline void setAccountIconColor(const QColor &value) {
        _accountIconColor = value;
        emit accountIconColorChanged();
    }

    inline QSize driveIconSize() const { return _driveIconSize; }
    inline void setDriveIconSize(const QSize &size) {
        _driveIconSize = size;
        emit driveIconSizeChanged();
    }

    inline QSize statusIconSize() const { return _statusIconSize; }
    inline void setStatusIconSize(const QSize &size) {
        _statusIconSize = size;
        emit statusIconSizeChanged();
    }

    inline QColor backgroundColor() const { return _backgroundColor; }
    inline void setBackgroundColor(const QColor &value) { _backgroundColor = value; }

    inline QColor backgroundColorSelection() const { return _backgroundColorSelection; }
    inline void setBackgroundColorSelection(const QColor &value) { _backgroundColorSelection = value; }

signals:
    void runSync(const QString &accountId);
    void pauseSync(const QString &accountId);
    void resumeSync(const QString &accountId);
    void remove(const QString &accountId);
    void accountIconSizeChanged();
    void accountIconColorChanged();
    void driveIconSizeChanged();
    void statusIconSizeChanged();

private:
    AccountItem _item;
    QSize _accountIconSize;
    QColor _accountIconColor;
    QSize _driveIconSize;
    QSize _statusIconSize;
    QColor _backgroundColor;
    QColor _backgroundColorSelection;
    QLabel *_accountIconLabel;
    QLabel *_accountNameLabel;
    QLabel *_statusLabel;
    CustomToolButton *_menuButton;

    void paintEvent(QPaintEvent *event) override;
    bool event(QEvent *event) override;

    QIcon getIconWithStatus();
    void setAccountIcon();

private slots:
    void onAccountIconSizeChanged();
    void onAccountIconColorChanged();
    void onDriveIconSizeChanged();
    void onStatusIconSizeChanged();
    void onMenuButtonClicked();
    void onSeeSyncErrorsTriggered();
    void onSyncTriggered();
    void onPauseTriggered();
    void onResumeTriggered();
    void onParametersTriggered();
    void onManageOfferTriggered();
    void onRemoveTriggered();
};

}


