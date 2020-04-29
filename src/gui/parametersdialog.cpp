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

#define CONSOLE_DEBUG
#ifdef CONSOLE_DEBUG
#include <iostream>
#endif

#include "parametersdialog.h"
#include "accountmanager.h"
#include "folderman.h"

#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QLoggingCategory>
#include <QPushButton>
#include <QVBoxLayout>

namespace KDC {

Q_LOGGING_CATEGORY(lcParametersDialog, "parametersdialog", QtInfoMsg)

ParametersDialog::ParametersDialog(QWidget *parent)
    : QDialog(parent)
    , _backgroundMainColor(QColor())
    , _mainMenuBarWidget(nullptr)
    , _stackedWidget(nullptr)
{
    initUI();

    connect(OCC::FolderMan::instance(), &OCC::FolderMan::folderSyncStateChange, this, &ParametersDialog::onRefreshAccountList);
    connect(OCC::AccountManager::instance(), &OCC::AccountManager::accountAdded, this, &ParametersDialog::onRefreshAccountList);
    connect(OCC::AccountManager::instance(), &OCC::AccountManager::accountRemoved, this, &ParametersDialog::onRefreshAccountList);
}

void ParametersDialog::initUI()
{
    QVBoxLayout *mainVBox = new QVBoxLayout();
    mainVBox->setContentsMargins(0, 0, 0, 0);
    mainVBox->setSpacing(0);

    // Main menu bar
    _mainMenuBarWidget = new MainMenuBarWidget(this);
    mainVBox->addWidget(_mainMenuBarWidget);

    // Stacked widget
    _stackedWidget = new QStackedWidget(this);

    _drivesWidget = new DrivesWidget(this);
    _stackedWidget->insertWidget(StackedWidget::Drives, _drivesWidget);

    QLabel *notImplementedLabel = new QLabel(tr("Not implemented!"), this);
    notImplementedLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    notImplementedLabel->setObjectName("defaultTitleLabel");
    _stackedWidget->insertWidget(StackedWidget::Preferences, notImplementedLabel);

    mainVBox->addWidget(_stackedWidget);
    mainVBox->setStretchFactor(_stackedWidget, 1);

    setLayout(mainVBox);

    connect(_mainMenuBarWidget, &MainMenuBarWidget::drivesButtonClicked, this, &ParametersDialog::onDrivesButtonClicked);
    connect(_mainMenuBarWidget, &MainMenuBarWidget::preferencesButtonClicked, this, &ParametersDialog::onPreferencesButtonClicked);
}

void ParametersDialog::onRefreshAccountList()
{
#ifdef CONSOLE_DEBUG
    std::cout << QTime::currentTime().toString("hh:mm:ss").toStdString()
              << " - ParametersDialog::onRefreshAccountList" << std::endl;
#endif

    if (OCC::AccountManager::instance()->accounts().isEmpty()) {
        _accountInfoMap.clear();
        _drivesWidget->clear();
    }
    else {
        for (OCC::AccountStatePtr accountStatePtr : OCC::AccountManager::instance()->accounts()) {
            QString accountId = accountStatePtr->account()->id();
            auto accountInfoIt = _accountInfoMap.find(accountId);
            if (accountInfoIt == _accountInfoMap.end()) {
                // New account
                _accountInfoMap[accountId] = AccountInfo();
                accountInfoIt = _accountInfoMap.find(accountId);
            }

            // Set or update account name & color
            accountInfoIt->second._name = accountStatePtr->account()->driveName();
            accountInfoIt->second._color = accountStatePtr->account()->getDriveColor();

            OCC::Folder::Map folderMap = OCC::FolderMan::instance()->map();
            for (auto folderIt = folderMap.begin(); folderIt != folderMap.end(); folderIt++) {
                OCC::AccountPtr folderAccountPtr = folderIt.value()->accountState()->account();
                if (folderAccountPtr->id() == accountId) {
                    auto folderInfoIt = accountInfoIt->second._folderMap.find(folderIt.key());
                    if (folderInfoIt == accountInfoIt->second._folderMap.end()) {
                        // New folder
                        accountInfoIt->second._folderMap[folderIt.key()] =
                                new FolderInfo(folderIt.value()->shortGuiLocalPath(), folderIt.value()->path());
                        folderInfoIt = accountInfoIt->second._folderMap.find(folderIt.key());
                    }

                    folderInfoIt->second->_paused = folderIt.value()->syncPaused();
                    folderInfoIt->second->_unresolvedConflicts = folderIt.value()->syncResult().hasUnresolvedConflicts();
                    folderInfoIt->second->_status = folderIt.value()->syncResult().status();
                }
            }

            // Manage removed folders
            auto folderInfoIt = accountInfoIt->second._folderMap.begin();
            while (folderInfoIt != accountInfoIt->second._folderMap.end()) {
                if (folderMap.find(folderInfoIt->first) == folderMap.end()) {
                    folderInfoIt = accountInfoIt->second._folderMap.erase(folderInfoIt);
                }
                else {
                    folderInfoIt++;
                }
            }

            // Compute account status
            accountInfoIt->second.updateStatus();

            _drivesWidget->addOrUpdateDrive(accountInfoIt->first, accountInfoIt->second);
        }

        // Manage removed accounts
        auto accountStatusIt = _accountInfoMap.begin();
        while (accountStatusIt != _accountInfoMap.end()) {
            if (!OCC::AccountManager::instance()->getAccountFromId(accountStatusIt->first)) {
                _drivesWidget->removeDrive(accountStatusIt->first);
                accountStatusIt = _accountInfoMap.erase(accountStatusIt);
            }
            else {
                accountStatusIt++;
            }
        }
    }
}

void ParametersDialog::onDrivesButtonClicked()
{
    _stackedWidget->setCurrentIndex(StackedWidget::Drives);
}

void ParametersDialog::onPreferencesButtonClicked()
{
    _stackedWidget->setCurrentIndex(StackedWidget::Preferences);
}

}
