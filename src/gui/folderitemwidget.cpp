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

#include "folderitemwidget.h"
#include "menuwidget.h"
#include "menuitemwidget.h"
#include "guiutility.h"

#include <QBoxLayout>
#include <QWidgetAction>

namespace KDC {

static const int hSpacing = 10;
static const int vSpacing = 10;
static const int expandButtonVMargin = 10;
static const int statusIconSize = 20;

FolderItemWidget::FolderItemWidget(const QString &folderId, const FolderInfo *folderInfo, QWidget *parent)
    : QWidget(parent)
    , _folderId(folderId)
    , _folderInfo(folderInfo)
    , _expandButton(nullptr)
    , _menuButton(nullptr)
    , _statusIconLabel(nullptr)
    , _isExpanded(false)
{
    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(0, expandButtonVMargin, 0, expandButtonVMargin);
    mainLayout->setSpacing(hSpacing);
    setLayout(mainLayout);

    // Expand button
    QVBoxLayout *expandVBoxLayout = new QVBoxLayout();
    expandVBoxLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addLayout(expandVBoxLayout);

    _expandButton = new CustomToolButton(this);
    _expandButton->setObjectName("expandButton");
    expandVBoxLayout->addWidget(_expandButton);
    expandVBoxLayout->addStretch();

    // Folder detail
    QVBoxLayout *detailVBoxLayout = new QVBoxLayout();
    detailVBoxLayout->setContentsMargins(0, 0, 0, 0);
    detailVBoxLayout->setSpacing(vSpacing);
    mainLayout->addLayout(detailVBoxLayout);
    mainLayout->setStretchFactor(detailVBoxLayout, 1);

    QHBoxLayout *detailHBoxLayout = new QHBoxLayout();
    detailHBoxLayout->setContentsMargins(0, 0, 0, 0);
    detailHBoxLayout->setSpacing(hSpacing);
    detailVBoxLayout->addLayout(detailHBoxLayout);

    _statusIconLabel = new QLabel(this);
    detailHBoxLayout->addWidget(_statusIconLabel);

    QLabel *nameLabel = new QLabel(this);
    nameLabel->setObjectName("largeMediumBoldTextLabel");
    detailHBoxLayout->addWidget(nameLabel);
    detailHBoxLayout->addStretch();

    QLabel *synchroLabel = new QLabel(this);
    synchroLabel->setObjectName("descriptionLabel");
    detailVBoxLayout->addWidget(synchroLabel);

    // Menu button
    _menuButton = new CustomToolButton(this);
    _menuButton->setIconPath(":/client/resources/icons/actions/menu.svg");
    _menuButton->setToolTip(tr("More actions"));
    mainLayout->addWidget(_menuButton);

    if (folderInfo) {
        updateItem(folderInfo);
        setExpandButton();
        nameLabel->setText(folderInfo->_name);
        synchroLabel->setText(tr("Synchronized into %1").arg(folderInfo->_path));
    }

    connect(_menuButton, &CustomToolButton::clicked, this, &FolderItemWidget::onMenuButtonClicked);
    connect(_expandButton, &CustomToolButton::clicked, this, &FolderItemWidget::onExpandButtonClicked);
}

void FolderItemWidget::updateItem(const FolderInfo *folderInfo)
{
    if (folderInfo) {
        _statusIconLabel->setPixmap(
                    QIcon(OCC::Utility::getAccountStatusIconPath(folderInfo->_paused, folderInfo->_status))
                    .pixmap(QSize(statusIconSize, statusIconSize)));
    }
}

void FolderItemWidget::setExpandButton()
{
    if (_isExpanded) {
        _expandButton->setIconPath(":/client/resources/icons/actions/chevron-up.svg");
    }
    else {
        _expandButton->setIconPath(":/client/resources/icons/actions/chevron-down.svg");
    }
}

void FolderItemWidget::onMenuButtonClicked()
{
    MenuWidget *menu = new MenuWidget(MenuWidget::Menu, this);

    if (OCC::Utility::getSyncActionAvailable(_folderInfo->_paused, _folderInfo->_status)) {
        QWidgetAction *syncAction = new QWidgetAction(this);
        MenuItemWidget *syncMenuItemWidget = new MenuItemWidget(tr("Force synchronization"));
        syncMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/sync.svg");
        syncAction->setDefaultWidget(syncMenuItemWidget);
        connect(syncAction, &QWidgetAction::triggered, this, &FolderItemWidget::onSyncTriggered);
        menu->addAction(syncAction);
    }

    if (OCC::Utility::getPauseActionAvailable(_folderInfo->_paused, _folderInfo->_status)) {
        QWidgetAction *pauseAction = new QWidgetAction(this);
        MenuItemWidget *pauseMenuItemWidget = new MenuItemWidget(tr("Pause synchronization"));
        pauseMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/pause.svg");
        pauseAction->setDefaultWidget(pauseMenuItemWidget);
        connect(pauseAction, &QWidgetAction::triggered, this, &FolderItemWidget::onPauseTriggered);
        menu->addAction(pauseAction);
    }

    if (OCC::Utility::getResumeActionAvailable(_folderInfo->_paused, _folderInfo->_status)) {
        QWidgetAction *resumeAction = new QWidgetAction(this);
        MenuItemWidget *resumeMenuItemWidget = new MenuItemWidget(tr("Resume synchronization"));
        resumeMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/start.svg");
        resumeAction->setDefaultWidget(resumeMenuItemWidget);
        connect(resumeAction, &QWidgetAction::triggered, this, &FolderItemWidget::onResumeTriggered);
        menu->addAction(resumeAction);
    }

    menu->addSeparator();

    QWidgetAction *unsyncAction = new QWidgetAction(this);
    MenuItemWidget *unsyncMenuItemWidget = new MenuItemWidget(tr("Remove advanced synchronization"));
    unsyncMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/delete.svg");
    unsyncAction->setDefaultWidget(unsyncMenuItemWidget);
    connect(unsyncAction, &QWidgetAction::triggered, this, &FolderItemWidget::onUnsyncTriggered);
    menu->addAction(unsyncAction);

    menu->exec(QWidget::mapToGlobal(_menuButton->geometry().center()));
}

void FolderItemWidget::onExpandButtonClicked()
{
    _isExpanded = !_isExpanded;
    setExpandButton();
    emit displayFolderDetail(_folderId, _isExpanded);
}

void FolderItemWidget::onSyncTriggered()
{
    emit runSync(_folderId);
}

void FolderItemWidget::onPauseTriggered()
{
    emit pauseSync(_folderId);
}

void FolderItemWidget::onResumeTriggered()
{
    emit resumeSync(_folderId);
}

void FolderItemWidget::onUnsyncTriggered()
{
    emit unSync(_folderId);
}

}
