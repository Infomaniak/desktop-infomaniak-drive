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

#include <map>

#include <QListWidget>
#include <QListWidgetItem>
#include <QWidget>

namespace KDC {

class DrivesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DrivesWidget(QWidget *parent = nullptr);

    void clear();
    void addOrUpdateDrive(QString accountId, const AccountInfo &accountInfo);
    void removeDrive(QString accountId);

signals:
    void addDrive();
    void runSync(const QString &accountId);
    void pauseSync(const QString &accountId);
    void resumeSync(const QString &accountId);
    void manageOffer(const QString &accountId);
    void remove(const QString &accountId);
    void displayDriveParameters(const QString &accountId);

private:
    struct AccountInfoDrivesWidget : public AccountInfo {
        QListWidgetItem *_item;

        AccountInfoDrivesWidget();
        AccountInfoDrivesWidget(const AccountInfo &accountInfo);
    };

    QListWidget *_driveListWidget;
    std::map<QString, AccountInfoDrivesWidget> _driveMap;

private slots:
    void onAddDrive();
    void onRunSync(const QString &accountId);
    void onPauseSync(const QString &accountId);
    void onResumeSync(const QString &accountId);
    void onManageOffer(const QString &accountId);
    void onRemove(const QString &accountId);
    void onDisplayDriveParameters(const QString &accountId);
};

}

