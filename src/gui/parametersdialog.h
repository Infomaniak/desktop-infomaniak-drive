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

#include "customdialog.h"
#include "mainmenubarwidget.h"
#include "drivemenubarwidget.h"
#include "errorsmenubarwidget.h"
#include "drivepreferenceswidget.h"
#include "driveswidget.h"
#include "preferenceswidget.h"
#include "synchronizeditem.h"
#include "accountinfo.h"
#include "progressdispatcher.h"

#include <map>

#include <QByteArray>
#include <QColor>
#include <QDialog>
#include <QStackedWidget>
#include <QString>

namespace KDC {

class ParametersDialog : public CustomDialog
{
    Q_OBJECT

public:
    explicit ParametersDialog(QWidget *parent = nullptr);

    void openErrorPage(const QString &accountId);

signals:
    void addDrive();
    void setStyle(bool darkTheme);

private:
    enum Page {
        Main = 0,
        Drive,
        Errors
    };

    enum MainStackedWidget {
        Drives = 0,
        Preferences
    };

    struct AccountInfoParameters : public AccountInfo {
        QListWidget *_errorsListWidget;
        int _errorsListStackPosition;

        AccountInfoParameters();
        AccountInfoParameters(OCC::AccountState *accountState);
    };

    QString _currentAccountId;
    QColor _backgroundMainColor;
    QStackedWidget *_pageStackedWidget;
    MainMenuBarWidget *_mainMenuBarWidget;
    DriveMenuBarWidget *_driveMenuBarWidget;
    ErrorsMenuBarWidget *_errorsMenuBarWidget;
    QStackedWidget *_mainStackedWidget;
    DrivesWidget *_drivesWidget;
    PreferencesWidget *_preferencesWidget;
    DrivePreferencesWidget *_drivePreferencesWidget;
    QStackedWidget *_errorsStackedWidget;
    std::map<QString, AccountInfoParameters> _accountInfoMap;

    void initUI();
    QByteArray contents(const QString& path);

private slots:
    void onExit();
    void onRefreshAccountList();
    void onUpdateProgress(const QString &folderId, const OCC::ProgressInfo &progress);
    void onUpdateQuota(qint64 total, qint64 used);
    void onItemCompleted(const QString &folderId, const OCC::SyncFileItemPtr &item);
    void onDrivesButtonClicked();
    void onPreferencesButtonClicked();
    void onOpenHelp();
    void onAddDrive();
    void onRunSync(const QString &accountId);
    void onPauseSync(const QString &accountId);
    void onResumeSync(const QString &accountId);
    void onManageOffer(const QString &accountId);
    void onRemove(const QString &accountId);
    void onDisplayErrors(const QString &accountId);
    void onDisplayDriveParameters(const QString &accountId);
    void onSetStyle(bool darkTheme);
    void onDisplayDrivesList();
    void onSendLogs();
    void onOpenFolderItem(const QString &filePath);
    void onDebugReporterDone(bool retCode, const QString &debugId = QString());
};

}

