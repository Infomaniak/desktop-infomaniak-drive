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

#include "mainmenubarwidget.h"
#include "drivemenubarwidget.h"
#include "drivepreferenceswidget.h"
#include "driveswidget.h"
#include "preferenceswidget.h"
#include "accountinfo.h"
#include "progressdispatcher.h"

#include <map>

#include <QColor>
#include <QDialog>
#include <QStackedLayout>
#include <QStackedWidget>
#include <QString>

namespace KDC {

class ParametersDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ParametersDialog(QWidget *parent = nullptr);

signals:
    void addDrive();

private:
    enum Page {
        Main = 0,
        Drive
    };

    enum StackedWidget {
        Drives = 0,
        Preferences
    };

    QColor _backgroundMainColor;
    QStackedLayout *_pageStackedLayout;
    MainMenuBarWidget *_mainMenuBarWidget;
    DriveMenuBarWidget *_driveMenuBarWidget;
    QStackedWidget *_mainStackedWidget;
    DrivesWidget *_drivesWidget;
    PreferencesWidget *_preferencesWidget;
    DrivePreferencesWidget *_drivePreferencesWidget;
    std::map<QString, AccountInfo> _accountInfoMap;

    void initUI();

private slots:
    void onRefreshAccountList();
    void onUpdateProgress(const QString &folderId, const OCC::ProgressInfo &progress);
    void onDrivesButtonClicked();
    void onPreferencesButtonClicked();
    void onOpenHelp();
    void onAddDrive();
    void onRunSync(const QString &accountId);
    void onPauseSync(const QString &accountId);
    void onResumeSync(const QString &accountId);
    void onRemove(const QString &accountId);
    void onDisplayDriveParameters(const QString &accountId);
    void onDisplayDrivesList();
};

}

