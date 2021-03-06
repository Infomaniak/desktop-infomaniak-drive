/*
Infomaniak Drive
Copyright (C) 2021 christophe.larchier@infomaniak.com

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
#include "preferencesmenubarwidget.h"
#include "errorsmenubarwidget.h"
#include "drivepreferenceswidget.h"
#include "preferenceswidget.h"
#include "synchronizeditem.h"
#include "accountinfo.h"
#include "progressdispatcher.h"

#include <map>

#include <QByteArray>
#include <QColor>
#include <QDialog>
#include <QListWidget>
#include <QStackedWidget>
#include <QString>

namespace KDC {

class ParametersDialog : public CustomDialog
{
    Q_OBJECT

public:
    explicit ParametersDialog(QWidget *parent = nullptr);

    void openPreferencesPage();
    void openDriveErrorsPage(const QString &accountId);
    void openDriveParametersPage(const QString &accountId);
    int driveErrorCount(const QString &accountId) const;

signals:
    void addDrive();
    void setStyle(bool darkTheme);
    void newBigFolderDiscovered();

private:
    enum Page {
        Drive = 0,
        Preferences,
        Errors
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
    MainMenuBarWidget *_driveMenuBarWidget;
    PreferencesMenuBarWidget *_preferencesMenuBarWidget;
    ErrorsMenuBarWidget *_errorsMenuBarWidget;
    PreferencesWidget *_preferencesWidget;
    DrivePreferencesWidget *_drivePreferencesWidget;
    QScrollArea *_drivePreferencesScrollArea;
    QWidget *_noDrivePagewidget;
    QStackedWidget *_errorsStackedWidget;
    std::map<QString, AccountInfoParameters> _accountInfoMap;

    bool event(QEvent *event) override;

    void initUI();
    QByteArray contents(const QString& path);
    void reset();

private slots:
    void onExit();
    void onRefreshAccountList();
    void onUpdateProgress(const QString &folderId, const OCC::ProgressInfo &progress);
    void onUpdateQuota(qint64 total, qint64 used);
    void onItemCompleted(const QString &folderId, const OCC::SyncFileItemPtr &item);
    void onPreferencesButtonClicked();
    void onOpenHelp();
    void onAccountSelected(QString id);
    void onAddDrive();
    void onRemoveDrive(const QString &accountId);
    void onDisplayDriveErrors(const QString &accountId);
    void onDisplayDriveParameters();
    void onDisplayPreferences();
    void onSetStyle(bool darkTheme);
    void onUndecidedListsCleared();
    void onSendLogs();
    void onOpenFolderItem(const QString &filePath);
    void onDebugReporterDone(bool retCode, const QString &debugId = QString());
    void onNewBigFolderDiscovered(const QString &path);
};

}

