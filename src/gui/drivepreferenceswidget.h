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

#include "customcheckbox.h"
#include "accountinfo.h"
#include "folderman.h"

#include <map>

#include <QBoxLayout>
#include <QLabel>
#include <QMetaObject>
#include <QProgressBar>
#include <QString>
#include <QWidget>

namespace KDC {

class DrivePreferencesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DrivePreferencesWidget(QWidget *parent = nullptr);

    void setAccount(const std::map<QString, AccountInfo>::iterator accountInfoIt);
    void reset();
    void setUsedSize(qint64 totalSize, qint64 size);

signals:

private:
    QString _accountId;
    AccountInfo *_accountInfo;
    QProgressBar *_progressBar;
    QLabel *_progressLabel;
    CustomCheckBox *_smartSyncCheckBox;
    QLabel *_smartSyncDescriptionLabel;
    QWidget *_locationWidget;
    QBoxLayout *_locationLayout;
    QLabel *_accountAvatarLabel;
    QLabel *_accountNameLabel;
    QLabel *_accountMailLabel;
    CustomCheckBox *_notificationsCheckBox;

    void updateSmartSyncCheckBoxState();
    void resetDriveLocation();
    void updateDriveLocation();
    void updateAccountInfo();
    void askEnableSmartSync(const std::function<void(bool enable)> &callback);
    void askDisableSmartSync(const std::function<void(bool enable)> &callback);
    void switchVfsOn(OCC::Folder *folder, std::shared_ptr<QMetaObject::Connection> connection);
    void switchVfsOff(OCC::Folder *folder, std::shared_ptr<QMetaObject::Connection> connection);
    void insertIconToLocation(int position, const QIcon &icon, const QSize &size);
    void insertTextToLocation(int position, const QString &text);

private slots:
    void onDisplaySmartSyncInfo(const QString &link);
    void onErrorsWidgetClicked();
    void onSmartSyncCheckBoxClicked(bool checked = false);
    void onDriveFoldersWidgetClicked();
    void onLocalFoldersWidgetClicked();
    void onOtherDevicesWidgetClicked();
    void onNotificationsCheckBoxClicked(bool checked = false);
};

}

