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
#include <QPushButton>
#include <QWidgetAction>

namespace KDC {

static const int hSpacing = 10;
static const int vSpacing = 10;
static const int expandButtonVMargin = 5;
static const int statusIconSize = 20;
static const int folderNameMaxSize = 50;
static const int folderPathMaxSize = 50;

FolderItemWidget::FolderItemWidget(const QString &folderId, const FolderInfo *folderInfo, QWidget *parent)
    : QWidget(parent)
    , _folderId(folderId)
    , _folderInfo(folderInfo)
    , _expandButton(nullptr)
    , _menuButton(nullptr)
    , _statusIconLabel(nullptr)
    , _nameLabel(nullptr)
    , _updateWidget(nullptr)
    , _isExpanded(false)
{
    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(hSpacing);
    setLayout(mainLayout);

    // Expand button
    QVBoxLayout *expandVBoxLayout = new QVBoxLayout();
    expandVBoxLayout->setContentsMargins(0, expandButtonVMargin, 0, expandButtonVMargin);
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

    QVBoxLayout *folderVBoxLayout = new QVBoxLayout();
    folderVBoxLayout->setContentsMargins(0, 0, 0, 0);
    folderVBoxLayout->setSpacing(hSpacing);
    detailHBoxLayout->addLayout(folderVBoxLayout);

    QHBoxLayout *folderHBoxLayout = new QHBoxLayout();
    folderHBoxLayout->setContentsMargins(0, 0, 0, 0);
    folderHBoxLayout->setSpacing(hSpacing);
    folderVBoxLayout->addLayout(folderHBoxLayout);

    _statusIconLabel = new QLabel(this);
    folderHBoxLayout->addWidget(_statusIconLabel);

    _nameLabel = new QLabel(this);
    _nameLabel->setObjectName("largeMediumBoldTextLabel");
    folderHBoxLayout->addWidget(_nameLabel);
    folderHBoxLayout->addStretch();

    QLabel *synchroLabel = new QLabel(this);
    synchroLabel->setObjectName("descriptionLabel");
    folderVBoxLayout->addWidget(synchroLabel);

    // Menu button
    _menuButton = new CustomToolButton(this);
    _menuButton->setIconPath(":/client/resources/icons/actions/menu.svg");
    _menuButton->setToolTip(tr("More actions"));
    detailHBoxLayout->addWidget(_menuButton);

    if (_folderInfo) {
        updateItem(_folderInfo);
        setExpandButton();
        QString name = _folderInfo->_name;
        if (name.size() > folderNameMaxSize) {
            name = name.left(folderNameMaxSize) + "...";
        }
        _nameLabel->setText(name);

        QString path = _folderInfo->_path;
        if (path.size() > folderPathMaxSize) {
            path = path.left(folderPathMaxSize) + "...";
        }
        synchroLabel->setText(tr("Synchronized into <a style=\"%1\" href=\"ref\">%2</a>")
                              .arg(OCC::Utility::linkStyle)
                              .arg(path));
    }

    // Update widget
    _updateWidget = new QWidget(this);
    _updateWidget->setVisible(false);
    detailVBoxLayout->addWidget(_updateWidget);

    QHBoxLayout *updateHBoxLayout = new QHBoxLayout();
    updateHBoxLayout->setContentsMargins(0, 0, 0, 0);
    updateHBoxLayout->setSpacing(hSpacing);
    _updateWidget->setLayout(updateHBoxLayout);

    QLabel *saveLabel = new QLabel(this);
    saveLabel->setObjectName("subtitleLabel");
    saveLabel->setText(tr("Unselected folders will be deleted and will no longer be synchronized with this computer."));
    saveLabel->setWordWrap(true);
    updateHBoxLayout->addWidget(saveLabel);
    updateHBoxLayout->setStretchFactor(saveLabel, 1);

    QPushButton *cancelButton = new QPushButton(this);
    cancelButton->setObjectName("nondefaultsmallbutton");
    cancelButton->setFlat(true);
    cancelButton->setText(tr("Cancel"));
    updateHBoxLayout->addWidget(cancelButton);

    QPushButton *validateButton = new QPushButton(this);
    validateButton->setObjectName("defaultsmallbutton");
    validateButton->setFlat(true);
    validateButton->setText(tr("VALIDATE"));
    updateHBoxLayout->addWidget(validateButton);

    connect(_menuButton, &CustomToolButton::clicked, this, &FolderItemWidget::onMenuButtonClicked);
    connect(_expandButton, &CustomToolButton::clicked, this, &FolderItemWidget::onExpandButtonClicked);
    connect(cancelButton, &QPushButton::clicked, this, &FolderItemWidget::onCancelButtonClicked);
    connect(validateButton, &QPushButton::clicked, this, &FolderItemWidget::onValidateButtonClicked);
    connect(synchroLabel, &QLabel::linkActivated, this, &FolderItemWidget::onDisplaySmartSyncInfo);
    connect(this, &FolderItemWidget::displayFolderDetailCanceled, this, &FolderItemWidget::onDisplayFolderDetailCanceled);
}

void FolderItemWidget::updateItem(const FolderInfo *folderInfo)
{
    if (folderInfo) {
        _statusIconLabel->setPixmap(
                    QIcon(OCC::Utility::getAccountStatusIconPath(folderInfo->_paused, folderInfo->_status))
                    .pixmap(QSize(statusIconSize, statusIconSize)));
    }
}

void FolderItemWidget::setUpdateWidgetVisible(bool visible)
{
    _updateWidget->setVisible(visible);
}

void FolderItemWidget::setExpandButton()
{
    _expandButton->hide();
    if (_isExpanded) {
        _expandButton->setIconPath(":/client/resources/icons/actions/chevron-down.svg");
    }
    else {
        _expandButton->setIconPath(":/client/resources/icons/actions/chevron-right.svg");
    }
    _expandButton->show();
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
    MenuItemWidget *unsyncMenuItemWidget = new MenuItemWidget(tr("Remove synchronization"));
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

void FolderItemWidget::onCancelButtonClicked()
{
    emit cancelUpdate(_folderId);
}

void FolderItemWidget::onValidateButtonClicked()
{
    emit validateUpdate(_folderId);
}

void FolderItemWidget::onDisplaySmartSyncInfo(const QString &link)
{
    Q_UNUSED(link)

    emit openFolder(_folderInfo->_path);
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

void FolderItemWidget::onDisplayFolderDetailCanceled()
{
    _isExpanded = false;
    setExpandButton();
}

}
