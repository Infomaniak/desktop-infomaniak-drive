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

#include "customswitch.h"
#include "actionwidget.h"
#include "customtoolbutton.h"
#include "preferencesblocwidget.h"
#include "folderitemwidget.h"
#include "accountinfo.h"
#include "folderman.h"

#include <map>

#include <QBoxLayout>
#include <QColor>
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

    void setAccount(const QString &accountId, const AccountInfo *accountInfo, bool errors);
    void reset();

signals:
    void displayErrors(const QString &accountId);
    void errorAdded();

private:
    QString _accountId;
    const AccountInfo *_accountInfo;
    QVBoxLayout *_mainVBox;
    ActionWidget *_displayErrorsWidget;
    CustomSwitch *_smartSyncSwitch;
    QLabel *_smartSyncDescriptionLabel;
    QLabel *_accountAvatarLabel;
    QLabel *_accountNameLabel;
    QLabel *_accountMailLabel;
    CustomSwitch *_notificationsSwitch;
    int _foldersBeginIndex;

    void updateSmartSyncSwitchState();
    void updateAccountInfo();
    void askEnableSmartSync(const std::function<void(bool enable)> &callback);
    void askDisableSmartSync(const std::function<void(bool enable)> &callback);
    void switchVfsOn(OCC::Folder *folder, std::shared_ptr<QMetaObject::Connection> connection);
    void switchVfsOff(OCC::Folder *folder, std::shared_ptr<QMetaObject::Connection> connection);
    void resetFoldersBlocs();
    void updateFoldersBlocs();

private slots:
    void onDisplaySmartSyncInfo(const QString &link);
    void onErrorsWidgetClicked();
    void onSmartSyncSwitchClicked(bool checked = false);
    void onDriveFoldersWidgetClicked();
    void onNotificationsSwitchClicked(bool checked = false);
    void onErrorAdded();
    void onSyncTriggered(const QString &folderId);
    void onPauseTriggered(const QString &folderId);
    void onResumeTriggered(const QString &folderId);
    void onUnsyncTriggered(const QString &folderId);
    void onDisplayFolderDetail(const QString &folderId, bool display);
};

}

