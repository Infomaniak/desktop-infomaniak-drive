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
#include "progressdispatcher.h"
#include "guiutility.h"

#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

namespace KDC {

Q_LOGGING_CATEGORY(lcParametersDialog, "parametersdialog", QtInfoMsg)

ParametersDialog::ParametersDialog(QWidget *parent)
    : QDialog(parent)
    , _backgroundMainColor(QColor())
    , _mainMenuBarWidget(nullptr)
    , _stackedWidget(nullptr)
    , _drivesWidget(nullptr)
    , _preferencesWidget(nullptr)
{
    initUI();

    connect(OCC::FolderMan::instance(), &OCC::FolderMan::folderSyncStateChange,
            this, &ParametersDialog::onRefreshAccountList);
    connect(OCC::AccountManager::instance(), &OCC::AccountManager::accountAdded,
            this, &ParametersDialog::onRefreshAccountList);
    connect(OCC::AccountManager::instance(), &OCC::AccountManager::accountRemoved,
            this, &ParametersDialog::onRefreshAccountList);
    connect(OCC::ProgressDispatcher::instance(), &OCC::ProgressDispatcher::progressInfo,
            this, &ParametersDialog::onUpdateProgress);
}

void ParametersDialog::initUI()
{
    QVBoxLayout *mainVBox = new QVBoxLayout();
    mainVBox->setContentsMargins(0, 0, 0, 0);
    mainVBox->setSpacing(0);

    // Main menu bar
    _mainMenuBarWidget = new MainMenuBarWidget(this);
    mainVBox->addWidget(_mainMenuBarWidget);

    //
    // Stacked widget
    //
    _stackedWidget = new QStackedWidget(this);

    // Drives list
    _drivesWidget = new DrivesWidget(this);
    _stackedWidget->insertWidget(StackedWidget::Drives, _drivesWidget);

    // Preferences
    _preferencesWidget = new PreferencesWidget(this);

    QScrollArea *preferencesScrollArea = new QScrollArea(this);
    preferencesScrollArea->setWidget(_preferencesWidget);
    preferencesScrollArea->setWidgetResizable(true);

    _stackedWidget->insertWidget(StackedWidget::Preferences, preferencesScrollArea);

    mainVBox->addWidget(_stackedWidget);
    mainVBox->setStretchFactor(_stackedWidget, 1);

    setLayout(mainVBox);

    connect(_mainMenuBarWidget, &MainMenuBarWidget::drivesButtonClicked, this, &ParametersDialog::onDrivesButtonClicked);
    connect(_mainMenuBarWidget, &MainMenuBarWidget::preferencesButtonClicked, this, &ParametersDialog::onPreferencesButtonClicked);
    connect(_drivesWidget, &DrivesWidget::addDrive, this, &ParametersDialog::onAddDrive);
    connect(_drivesWidget, &DrivesWidget::runSync, this, &ParametersDialog::onRunSync);
    connect(_drivesWidget, &DrivesWidget::pauseSync, this, &ParametersDialog::onPauseSync);
    connect(_drivesWidget, &DrivesWidget::resumeSync, this, &ParametersDialog::onResumeSync);
    connect(_drivesWidget, &DrivesWidget::remove, this, &ParametersDialog::onRemove);
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
            if (accountStatePtr && !accountStatePtr->account().isNull()) {
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
                accountInfoIt->second._isSignedIn = !accountStatePtr->isSignedOut();

                OCC::Folder::Map folderMap = OCC::FolderMan::instance()->map();
                for (auto folderIt = folderMap.begin(); folderIt != folderMap.end(); folderIt++) {
                    if (folderIt.value() && folderIt.value()->accountState()) {
                        OCC::AccountPtr folderAccountPtr = folderIt.value()->accountState()->account();
                        if (!folderAccountPtr.isNull()) {
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
                        else {
                            qCDebug(lcParametersDialog) << "Null pointer!";
                            Q_ASSERT(false);
                        }
                    }
                    else {
                        qCDebug(lcParametersDialog) << "Null pointer!";
                        Q_ASSERT(false);
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
            else {
                qCDebug(lcParametersDialog) << "Null pointer!";
                Q_ASSERT(false);
            }
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

void ParametersDialog::onUpdateProgress(const QString &folderId, const OCC::ProgressInfo &progress)
{
    OCC::Folder *folder = OCC::FolderMan::instance()->folder(folderId);
    if (folder) {
#ifdef CONSOLE_DEBUG
        std::cout << QTime::currentTime().toString("hh:mm:ss").toStdString()
                  << " - ParametersDialog::onUpdateProgress folder: " << folder->path().toStdString() << std::endl;
#endif

        if (folder->accountState()) {
            OCC::AccountPtr account = folder->accountState()->account();
            if (!account.isNull()) {
                const auto accountInfoIt = _accountInfoMap.find(account->id());
                if (accountInfoIt != _accountInfoMap.end()) {
                    const auto folderInfoIt = accountInfoIt->second._folderMap.find(folderId);
                    if (folderInfoIt != accountInfoIt->second._folderMap.end()) {
                        FolderInfo *folderInfo = folderInfoIt->second;
                        if (folderInfo) {
                            folderInfo->_currentFile = progress.currentFile();
                            folderInfo->_totalFiles = qMax(progress.currentFile(), progress.totalFiles());
                            folderInfo->_completedSize = progress.completedSize();
                            folderInfo->_totalSize = qMax(progress.completedSize(), progress.totalSize());
                            folderInfo->_estimatedRemainingTime = progress.totalProgress().estimatedEta;
                            folderInfo->_paused = folder->syncPaused();
                            folderInfo->_unresolvedConflicts = folder->syncResult().hasUnresolvedConflicts();
                            folderInfo->_status = folder->syncResult().status();
                        }
                        else {
                            qCDebug(lcParametersDialog) << "Null pointer!";
                            Q_ASSERT(false);
                        }
                    }

                    // Compute account status
                    accountInfoIt->second.updateStatus();

                    _drivesWidget->addOrUpdateDrive(account->id(), accountInfoIt->second);
                }
            }
            else {
                qCDebug(lcParametersDialog) << "Null pointer!";
                Q_ASSERT(false);
            }
        }
        else {
            qCDebug(lcParametersDialog) << "Null pointer!";
            Q_ASSERT(false);
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

void ParametersDialog::onAddDrive()
{
    emit addDrive();
}

void ParametersDialog::onRunSync(const QString &accountId)
{
    OCC::Utility::runSync(accountId);
}

void ParametersDialog::onPauseSync(const QString &accountId)
{
    OCC::Utility::pauseSync(accountId, true);
}

void ParametersDialog::onResumeSync(const QString &accountId)
{
    OCC::Utility::pauseSync(accountId, false);
}

void ParametersDialog::onRemove(const QString &accountId)
{
    OCC::AccountManager *accountManager = OCC::AccountManager::instance();
    OCC::AccountStatePtr accountStatePtr = accountManager->getAccountStateFromId(accountId);
    if (accountStatePtr.data()) {
        QMessageBox messageBox(QMessageBox::Question,
            tr("Confirm Account Removal"),
            tr("<p>Do you really want to remove the connection to the account <i>%1</i>?</p>"
               "<p><b>Note:</b> This will <b>not</b> delete any files.</p>")
                .arg(accountStatePtr->account()->driveName()),
            QMessageBox::NoButton,
            this);
        QPushButton *yesButton = messageBox.addButton(tr("Remove connection"), QMessageBox::YesRole);
        messageBox.addButton(tr("Cancel"), QMessageBox::NoRole);

        messageBox.exec();
        if (messageBox.clickedButton() != yesButton) {
            return;
        }

        accountManager->deleteAccount(accountStatePtr.data());
        accountManager->save();
    }
}

}
