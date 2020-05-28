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

#include "drivemenubarwidget.h"
#include "customtoolbutton.h"
#include "menuitemwidget.h"
#include "menuwidget.h"
#include "guiutility.h"

#include <QSize>
#include <QWidgetAction>

namespace KDC {

static const int hMargin = 15;
static const int vMargin = 10;
static const int hButtonsSpacing = 10;
static const int driveLogoIconSize = 24;

DriveMenuBarWidget::DriveMenuBarWidget(QWidget *parent)
    : HalfRoundRectWidget(parent)
    , _accountId(QString())
    , _accountInfo(nullptr)
    , _accountIconLabel(nullptr)
    , _accountNameLabel(nullptr)
    , _menuButton(nullptr)
{
    setContentsMargins(hMargin, vMargin, hMargin, vMargin);
    setSpacing(0);

    CustomToolButton *backButton = new CustomToolButton(this);
    backButton->setIconPath(":/client/resources/icons/actions/arrow-left.svg");
    backButton->setToolTip(tr("Back to drive list"));
    addWidget(backButton);

    addSpacing(hButtonsSpacing);

    _accountIconLabel = new QLabel(this);
    addWidget(_accountIconLabel);

    addSpacing(hButtonsSpacing);

    _accountNameLabel = new QLabel(this);
    _accountNameLabel->setObjectName("titleLabel");
    addWidget(_accountNameLabel);

    addStretch();

    _menuButton = new CustomToolButton(this);
    _menuButton->setObjectName("backButton");
    _menuButton->setIconPath(":/client/resources/icons/actions/menu.svg");
    _menuButton->setToolTip(tr("More actions"));
    _menuButton->setEnabled(false);
    addWidget(_menuButton);

    connect(backButton, &CustomToolButton::clicked, this, &DriveMenuBarWidget::onBackButtonClicked);
    connect(_menuButton, &CustomToolButton::clicked, this, &DriveMenuBarWidget::onMenuButtonClicked);
}

void DriveMenuBarWidget::setAccount(const QString &accountId, const AccountInfo *accountInfo)
{
    _accountId = accountId;
    _accountInfo = accountInfo;
    _accountIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/drive.svg",
                                                                _accountInfo->_color).
                                 pixmap(QSize(driveLogoIconSize, driveLogoIconSize)));
    _accountNameLabel->setText(_accountInfo->_name);
    _menuButton->setEnabled(true);
}

void DriveMenuBarWidget::reset()
{
    _accountId = QString();
    _accountInfo = nullptr;
    _accountNameLabel->setText(QString());
    _menuButton->setEnabled(false);
}

void DriveMenuBarWidget::onBackButtonClicked(bool checked)
{
    Q_UNUSED(checked)

    emit backButtonClicked();
}

void DriveMenuBarWidget::onMenuButtonClicked(bool checked)
{
    Q_UNUSED(checked)

    if (!_accountInfo) {
        return;
    }

    MenuWidget *menu = new MenuWidget(MenuWidget::Menu, this);

    if (OCC::Utility::getSyncActionAvailable(_accountInfo->_paused, _accountInfo->_status)) {
        QWidgetAction *syncAction = new QWidgetAction(this);
        MenuItemWidget *syncMenuItemWidget = new MenuItemWidget(tr("Force synchronization"));
        syncMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/sync.svg");
        syncAction->setDefaultWidget(syncMenuItemWidget);
        connect(syncAction, &QWidgetAction::triggered, this, &DriveMenuBarWidget::onSyncTriggered);
        menu->addAction(syncAction);
    }

    if (OCC::Utility::getPauseActionAvailable(_accountInfo->_paused, _accountInfo->_status)) {
        QWidgetAction *pauseAction = new QWidgetAction(this);
        MenuItemWidget *pauseMenuItemWidget = new MenuItemWidget(tr("Pause synchronization"));
        pauseMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/pause.svg");
        pauseAction->setDefaultWidget(pauseMenuItemWidget);
        connect(pauseAction, &QWidgetAction::triggered, this, &DriveMenuBarWidget::onPauseTriggered);
        menu->addAction(pauseAction);
    }

    if (OCC::Utility::getResumeActionAvailable(_accountInfo->_paused, _accountInfo->_status)) {
        QWidgetAction *resumeAction = new QWidgetAction(this);
        MenuItemWidget *resumeMenuItemWidget = new MenuItemWidget(tr("Resume synchronization"));
        resumeMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/start.svg");
        resumeAction->setDefaultWidget(resumeMenuItemWidget);
        connect(resumeAction, &QWidgetAction::triggered, this, &DriveMenuBarWidget::onResumeTriggered);
        menu->addAction(resumeAction);
    }

    QWidgetAction *manageOfferAction = new QWidgetAction(this);
    MenuItemWidget *manageOfferMenuItemWidget = new MenuItemWidget(tr("Manage my offer in Infomaniak manager"));
    manageOfferMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/webview.svg");
    manageOfferAction->setDefaultWidget(manageOfferMenuItemWidget);
    connect(manageOfferAction, &QWidgetAction::triggered, this, &DriveMenuBarWidget::onManageOfferTriggered);
    menu->addAction(manageOfferAction);

    menu->addSeparator();

    QWidgetAction *signOutAction = new QWidgetAction(this);
    MenuItemWidget *signOutMenuItemWidget = new MenuItemWidget(tr("Sign out"));
    signOutMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/logout.svg");
    signOutAction->setDefaultWidget(signOutMenuItemWidget);
    connect(signOutAction, &QWidgetAction::triggered, this, &DriveMenuBarWidget::onRemoveTriggered);
    menu->addAction(signOutAction);

    menu->exec(QWidget::mapToGlobal(_menuButton->geometry().center()));
}

void DriveMenuBarWidget::onSyncTriggered()
{
    emit runSync(_accountId);
}

void DriveMenuBarWidget::onPauseTriggered()
{
    emit pauseSync(_accountId);
}

void DriveMenuBarWidget::onResumeTriggered()
{
    emit resumeSync(_accountId);
}

void DriveMenuBarWidget::onManageOfferTriggered()
{
    emit manageOffer(_accountId);
}

void DriveMenuBarWidget::onRemoveTriggered()
{
    emit remove(_accountId);
}

}
