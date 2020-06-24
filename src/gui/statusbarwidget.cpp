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

#include "statusbarwidget.h"
#include "menuitemwidget.h"
#include "menuwidget.h"
#include "guiutility.h"
#include "common/utility.h"

#include <QVBoxLayout>
#include <QStringList>
#include <QWidgetAction>

namespace KDC {

static const int hMargin = 15;
static const int vMargin = 15;
static const int statusIconSize = 24;

StatusBarWidget::StatusBarWidget(QWidget *parent)
    : HalfRoundRectWidget(parent)
    , _status(OCC::SyncResult::Undefined)
    , _accountInfo(nullptr)
    , _statusIconLabel(nullptr)
    , _statusLabel(nullptr)
    , _pauseButton(nullptr)
    , _resumeButton(nullptr)
    , _syncButton(nullptr)
{
    setContentsMargins(hMargin, vMargin, hMargin, vMargin);

    _statusIconLabel = new QLabel(this);
    _statusIconLabel->setVisible(false);
    addWidget(_statusIconLabel);

    QVBoxLayout *vBoxLayout = new QVBoxLayout();
    vBoxLayout->setContentsMargins(0, 0, 0, 0);
    vBoxLayout->setSpacing(0);
    addLayout(vBoxLayout);
    setStretchFactor(vBoxLayout, 1);

    _statusLabel = new QLabel(this);
    _statusLabel->setObjectName("statusLabel");
    _statusLabel->setWordWrap(true);
    vBoxLayout->addWidget(_statusLabel);

    addStretch();

    _pauseButton = new CustomToolButton(this);
    _pauseButton->setIconPath(":/client/resources/icons/actions/pause.svg");
    _pauseButton->setToolTip(tr("Pause synchronization"));
    _pauseButton->setVisible(false);
    addWidget(_pauseButton);

    _resumeButton = new CustomToolButton(this);
    _resumeButton->setIconPath(":/client/resources/icons/actions/start.svg");
    _resumeButton->setToolTip(tr("Resume synchronization"));
    _resumeButton->setVisible(false);
    addWidget(_resumeButton);

    _syncButton = new CustomToolButton(this);
    _syncButton->setIconPath(":/client/resources/icons/actions/sync-circle.svg");
    _syncButton->setToolTip(tr("Force synchronization"));
    _syncButton->setVisible(false);
    addWidget(_syncButton);

    connect(_pauseButton, &CustomToolButton::clicked, this, &StatusBarWidget::onPauseClicked);
    connect(_resumeButton, &CustomToolButton::clicked, this, &StatusBarWidget::onResumeClicked);
    connect(_syncButton, &CustomToolButton::clicked, this, &StatusBarWidget::onSyncClicked);
}

void StatusBarWidget::setStatus(bool paused, bool unresolvedConflicts, OCC::SyncResult::Status status,
                                qint64 currentFile, qint64 totalFiles, qint64 estimatedRemainingTime)
{
    _statusIconLabel->setPixmap(QIcon(OCC::Utility::getFolderStatusIconPath(paused, status))
                                .pixmap(QSize(statusIconSize, statusIconSize)));

    QString statusText = OCC::Utility::getFolderStatusText(paused, unresolvedConflicts, status,
                                                           currentFile, totalFiles, estimatedRemainingTime);
    _statusLabel->setText(statusText);
    _statusIconLabel->setVisible(true);
    if (_accountInfo) {
        _pauseButton->setVisible(OCC::Utility::getPauseActionAvailable(_accountInfo->_paused, _accountInfo->_status));
        _resumeButton->setVisible(OCC::Utility::getResumeActionAvailable(_accountInfo->_paused, _accountInfo->_status));
        _syncButton->setVisible(OCC::Utility::getSyncActionAvailable(_accountInfo->_paused, _accountInfo->_status));
    }
    else {
        _pauseButton->setVisible(false);
        _resumeButton->setVisible(false);
        _syncButton->setVisible(false);
    }

    repaint();

    connect(_statusLabel, &QLabel::linkActivated, this, &StatusBarWidget::onLinkActivated);
}

void StatusBarWidget::setCurrentAccount(const AccountInfo *accountInfo)
{
    _accountInfo = accountInfo;
}

void StatusBarWidget::setSeveralDrives(bool severalDrives)
{
    _severalDrives = severalDrives;
    _pauseButton->setWithMenu(_severalDrives);
    _resumeButton->setWithMenu(_severalDrives);
    _syncButton->setWithMenu(_severalDrives);
}

void StatusBarWidget::reset()
{
    _accountInfo = nullptr;
    setStatus(false, false, OCC::SyncResult::Undefined);
}

void StatusBarWidget::onLinkActivated(const QString &link)
{
    emit linkActivated(link);
}

void StatusBarWidget::onPauseClicked()
{
    bool resetButtons = false;

    if (_severalDrives) {
        MenuWidget *menu = new MenuWidget(MenuWidget::Menu, this);

        // Pause
        QWidgetAction *pauseAction = new QWidgetAction(this);
        MenuItemWidget *pauseMenuItemWidget = new MenuItemWidget(tr("Pause kDrive \"%1\" synchronization")
                                                                 .arg(_accountInfo ? _accountInfo->_name : QString()));
        pauseMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/pause.svg");
        pauseAction->setDefaultWidget(pauseMenuItemWidget);
        connect(pauseAction, &QWidgetAction::triggered, this, &StatusBarWidget::onPauseSync);
        menu->addAction(pauseAction);

        // Pause folder
        if (_accountInfo->_folderMap.size() > 1) {
            QWidgetAction *pauseFoldersAction = new QWidgetAction(this);
            MenuItemWidget *pauseFoldersMenuItemWidget = new MenuItemWidget(tr("Pause folder synchronization"));
            pauseFoldersMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/pause.svg");
            pauseFoldersMenuItemWidget->setHasSubmenu(true);
            pauseFoldersAction->setDefaultWidget(pauseFoldersMenuItemWidget);
            menu->addAction(pauseFoldersAction);

            // Pause folder submenu
            MenuWidget *submenu = new MenuWidget(MenuWidget::Submenu, menu);

            QActionGroup *pauseFolderActionGroup = new QActionGroup(this);
            pauseFolderActionGroup->setExclusive(true);

            QWidgetAction *pauseFolderAction;
            for (auto const &folderInfoElt : _accountInfo->_folderMap) {
                pauseFolderAction = new QWidgetAction(this);
                pauseFolderAction->setProperty(MenuWidget::actionTypeProperty.c_str(), folderInfoElt.first);
                MenuItemWidget *pauseFolderMenuItemWidget = new MenuItemWidget(folderInfoElt.second->_name);
                pauseFolderMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/drive.svg");
                pauseFolderAction->setDefaultWidget(pauseFolderMenuItemWidget);
                connect(pauseFolderAction, &QWidgetAction::triggered, this, &StatusBarWidget::onPauseFolderSync);
                pauseFolderActionGroup->addAction(pauseFolderAction);
            }

            submenu->addActions(pauseFolderActionGroup->actions());
            pauseFoldersAction->setMenu(submenu);
        }

        // Pause all
        QWidgetAction *pauseAllAction = new QWidgetAction(this);
        MenuItemWidget *pauseAllMenuItemWidget = new MenuItemWidget(tr("Pause all kDrives synchronization"));
        pauseAllMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/pause.svg");
        pauseAllAction->setDefaultWidget(pauseAllMenuItemWidget);
        connect(pauseAllAction, &QWidgetAction::triggered, this, &StatusBarWidget::onPauseAllSync);
        menu->addSeparator();
        menu->addAction(pauseAllAction);

        if (menu->exec(QWidget::mapToGlobal(_pauseButton->geometry().center()))) {
            resetButtons = true;
        }
    }
    else {
        emit pauseSync(ActionType::Drive);
        resetButtons = true;
    }

    if (resetButtons) {
        // Status is NOT updated after a sync pause
        _pauseButton->setVisible(false);
        _resumeButton->setVisible(true);
        _syncButton->setVisible(false);
    }
}

void StatusBarWidget::onResumeClicked()
{
    bool resetButtons = false;

    if (_severalDrives) {
        MenuWidget *menu = new MenuWidget(MenuWidget::Menu, this);

        // Resume
        QWidgetAction *resumeAction = new QWidgetAction(this);
        MenuItemWidget *resumeMenuItemWidget = new MenuItemWidget(tr("Resume kDrive \"%1\" synchronization")
                                                                  .arg(_accountInfo ? _accountInfo->_name : QString()));
        resumeMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/start.svg");
        resumeAction->setDefaultWidget(resumeMenuItemWidget);
        connect(resumeAction, &QWidgetAction::triggered, this, &StatusBarWidget::onResumeSync);
        menu->addAction(resumeAction);

        // Resume folder
        if (_accountInfo->_folderMap.size() > 1) {
            QWidgetAction *resumeFoldersAction = new QWidgetAction(this);
            MenuItemWidget *resumeFoldersMenuItemWidget = new MenuItemWidget(tr("Resume folder synchronization"));
            resumeFoldersMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/start.svg");
            resumeFoldersMenuItemWidget->setHasSubmenu(true);
            resumeFoldersAction->setDefaultWidget(resumeFoldersMenuItemWidget);
            menu->addAction(resumeFoldersAction);

            // Resume folder submenu
            MenuWidget *submenu = new MenuWidget(MenuWidget::Submenu, menu);

            QActionGroup *resumeFolderActionGroup = new QActionGroup(this);
            resumeFolderActionGroup->setExclusive(true);

            QWidgetAction *resumeFolderAction;
            for (auto const &folderInfoElt : _accountInfo->_folderMap) {
                resumeFolderAction = new QWidgetAction(this);
                resumeFolderAction->setProperty(MenuWidget::actionTypeProperty.c_str(), folderInfoElt.first);
                MenuItemWidget *resumeFolderMenuItemWidget = new MenuItemWidget(folderInfoElt.second->_name);
                resumeFolderMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/drive.svg");
                resumeFolderAction->setDefaultWidget(resumeFolderMenuItemWidget);
                connect(resumeFolderAction, &QWidgetAction::triggered, this, &StatusBarWidget::onResumeFolderSync);
                resumeFolderActionGroup->addAction(resumeFolderAction);
            }

            submenu->addActions(resumeFolderActionGroup->actions());
            resumeFoldersAction->setMenu(submenu);
        }

        // Resume all
        QWidgetAction *resumeAllAction = new QWidgetAction(this);
        MenuItemWidget *resumeAllMenuItemWidget = new MenuItemWidget(tr("Resume all kDrives synchronization"));
        resumeAllMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/start.svg");
        resumeAllAction->setDefaultWidget(resumeAllMenuItemWidget);
        connect(resumeAllAction, &QWidgetAction::triggered, this, &StatusBarWidget::onResumeAllSync);
        menu->addSeparator();
        menu->addAction(resumeAllAction);

        if (menu->exec(QWidget::mapToGlobal(_resumeButton->geometry().center()))) {
            resetButtons = true;
        }
    }
    else {
        emit resumeSync(ActionType::Drive);
        resetButtons = true;
    }

    if (resetButtons) {
        // Status is updated after a sync resume
        _pauseButton->setVisible(false);
        _resumeButton->setVisible(false);
        _syncButton->setVisible(false);
    }
}

void StatusBarWidget::onSyncClicked()
{
    bool resetButtons = false;

    if (_severalDrives) {
        MenuWidget *menu = new MenuWidget(MenuWidget::Menu, this);

        // Force sync
        QWidgetAction *runAction = new QWidgetAction(this);
        MenuItemWidget *runMenuItemWidget = new MenuItemWidget(tr("Force kDrive \"%1\" synchronization")
                                                               .arg(_accountInfo ? _accountInfo->_name : QString()));
        runMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/sync-circle.svg");
        runAction->setDefaultWidget(runMenuItemWidget);
        connect(runAction, &QWidgetAction::triggered, this, &StatusBarWidget::onRunSync);
        menu->addAction(runAction);

        // Force sync folder
        if (_accountInfo->_folderMap.size() > 1) {
            QWidgetAction *runFoldersAction = new QWidgetAction(this);
            MenuItemWidget *runFoldersMenuItemWidget = new MenuItemWidget(tr("Force folder synchronization"));
            runFoldersMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/sync-circle.svg");
            runFoldersMenuItemWidget->setHasSubmenu(true);
            runFoldersAction->setDefaultWidget(runFoldersMenuItemWidget);
            menu->addAction(runFoldersAction);

            // Force sync folder submenu
            MenuWidget *submenu = new MenuWidget(MenuWidget::Submenu, menu);

            QActionGroup *runFolderActionGroup = new QActionGroup(this);
            runFolderActionGroup->setExclusive(true);

            QWidgetAction *runFolderAction;
            for (auto const &folderInfoElt : _accountInfo->_folderMap) {
                runFolderAction = new QWidgetAction(this);
                runFolderAction->setProperty(MenuWidget::actionTypeProperty.c_str(), folderInfoElt.first);
                MenuItemWidget *runFolderMenuItemWidget = new MenuItemWidget(folderInfoElt.second->_name);
                runFolderMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/drive.svg");
                runFolderAction->setDefaultWidget(runFolderMenuItemWidget);
                connect(runFolderAction, &QWidgetAction::triggered, this, &StatusBarWidget::onRunFolderSync);
                runFolderActionGroup->addAction(runFolderAction);
            }

            submenu->addActions(runFolderActionGroup->actions());
            runFoldersAction->setMenu(submenu);
        }

        // Force sync all
        QWidgetAction *runAllAction = new QWidgetAction(this);
        MenuItemWidget *runAllMenuItemWidget = new MenuItemWidget(tr("Force all kDrives synchronization"));
        runAllMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/sync-circle.svg");
        runAllAction->setDefaultWidget(runAllMenuItemWidget);
        connect(runAllAction, &QWidgetAction::triggered, this, &StatusBarWidget::onRunAllSync);
        menu->addSeparator();
        menu->addAction(runAllAction);

        menu->exec(QWidget::mapToGlobal(_syncButton->geometry().center()));
    }
    else {
        emit runSync(ActionType::Drive);
    }
}

void StatusBarWidget::onPauseSync()
{
    emit pauseSync(ActionType::Drive);
}

void StatusBarWidget::onPauseFolderSync()
{
    QString folderId = qvariant_cast<QString>(sender()->property(MenuWidget::actionTypeProperty.c_str()));
    emit pauseSync(ActionType::Folder, folderId);
}

void StatusBarWidget::onPauseAllSync()
{
    emit pauseSync(ActionType::AllDrives);
}

void StatusBarWidget::onResumeSync()
{
    emit resumeSync(ActionType::Drive);
}

void StatusBarWidget::onResumeFolderSync()
{
    QString folderId = qvariant_cast<QString>(sender()->property(MenuWidget::actionTypeProperty.c_str()));
    emit resumeSync(ActionType::Folder, folderId);
}

void StatusBarWidget::onResumeAllSync()
{
    emit resumeSync(ActionType::AllDrives);
}


void StatusBarWidget::onRunSync()
{
    emit resumeSync(ActionType::Drive);
}

void StatusBarWidget::onRunFolderSync()
{
    QString folderId = qvariant_cast<QString>(sender()->property(MenuWidget::actionTypeProperty.c_str()));
    emit runSync(ActionType::Folder, folderId);
}

void StatusBarWidget::onRunAllSync()
{
    emit runSync(ActionType::AllDrives);
}

}
