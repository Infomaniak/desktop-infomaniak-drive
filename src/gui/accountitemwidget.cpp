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

#include "accountitemwidget.h"
#include "menuwidget.h"
#include "menuitemwidget.h"
#include "guiutility.h"

#include <QGraphicsColorizeEffect>
#include <QGraphicsScene>
#include <QGraphicsSvgItem>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QWidgetAction>

namespace KDC {

static const int cornerRadius = 10;
static const int hMargin= 20;
static const int vMargin = 5;
static const int boxHMargin= 15;
static const int boxVMargin = 5;
static const int boxSpacing = 15;
static const int textBoxHMargin= 0;
static const int textBoxVMargin = 10;
static const int driveNameMaxSize = 50;
static const int shadowBlurRadius = 20;

AccountItemWidget::AccountItemWidget(const QString &accountId, QWidget *parent)
    : QWidget(parent)
    , _item(accountId)
    , _backgroundColor(QColor())
    , _accountIconLabel(nullptr)
{
    setContentsMargins(hMargin, vMargin, hMargin, vMargin);

    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->setContentsMargins(boxHMargin, boxVMargin, boxHMargin, boxVMargin);
    hbox->setSpacing(boxSpacing);
    setLayout(hbox);

    _accountIconLabel = new QLabel(this);
    hbox->addWidget(_accountIconLabel);

    QVBoxLayout *vboxText = new QVBoxLayout();
    vboxText->setContentsMargins(textBoxHMargin, textBoxVMargin, textBoxHMargin, textBoxVMargin);
    vboxText->setSpacing(0);

    _accountNameLabel = new QLabel(this);
    _accountNameLabel->setObjectName("accountNameLabel");
    vboxText->addWidget(_accountNameLabel);

    _statusLabel = new QLabel(this);
    _statusLabel->setObjectName("statusLabel");
    vboxText->addWidget(_statusLabel);
    hbox->addLayout(vboxText);

    _menuButton = new CustomToolButton(this);
    _menuButton->setIconPath(":/client/resources/icons/actions/menu.svg");
    _menuButton->setToolTip(tr("More actions"));
    hbox->addWidget(_menuButton);
    hbox->setStretchFactor(vboxText, 1);

    // Shadow
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
    effect->setBlurRadius(shadowBlurRadius);
    effect->setOffset(0);
    setGraphicsEffect(effect);

    connect(this, &AccountItemWidget::accountIconSizeChanged, this, &AccountItemWidget::onAccountIconSizeChanged);
    connect(this, &AccountItemWidget::accountIconColorChanged, this, &AccountItemWidget::onAccountIconColorChanged);
    connect(this, &AccountItemWidget::driveIconSizeChanged, this, &AccountItemWidget::onDriveIconSizeChanged);
    connect(this, &AccountItemWidget::statusIconSizeChanged, this, &AccountItemWidget::onStatusIconSizeChanged);
    connect(_menuButton, &CustomToolButton::clicked, this, &AccountItemWidget::onMenuButtonClicked);
}

void AccountItemWidget::updateItem(const AccountInfo &accountInfo)
{
    _item.accountInfo() = accountInfo;

    setAccountIcon();

    QString accountName = _item.accountInfo()._name;
    if (accountName.size() > driveNameMaxSize) {
        accountName = accountName.left(driveNameMaxSize) + "...";
    }
    _accountNameLabel->setText(accountName);

    _statusLabel->setText(OCC::Utility::getAccountStatusText(_item.accountInfo()._paused,
                                                             _item.accountInfo()._unresolvedConflicts,
                                                             _item.accountInfo()._status));
}

void AccountItemWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    // Shadow
    QGraphicsDropShadowEffect *effect = qobject_cast<QGraphicsDropShadowEffect *>(graphicsEffect());
    if (effect && effect->color() != OCC::Utility::getShadowColor()) {
        effect->setColor(OCC::Utility::getShadowColor());
        effect->update();
    }

    // Draw round rectangle
    QPainterPath painterPath2;
    painterPath2.addRoundedRect(rect().marginsRemoved(QMargins(hMargin, vMargin, hMargin, vMargin)),
                                cornerRadius, cornerRadius);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(backgroundColor());
    painter.drawPath(painterPath2);
}

bool AccountItemWidget::event(QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        onParametersTriggered();
    }
    return QWidget::event(event);
}

QIcon AccountItemWidget::getIconWithStatus()
{
    QGraphicsSvgItem *accountItem = new QGraphicsSvgItem(":/client/resources/icons/actions/drive.svg");
    QGraphicsSvgItem *statusItem =
            new QGraphicsSvgItem(OCC::Utility::getAccountStatusIconPath(_item.accountInfo()._paused,
                                                                        _item.accountInfo()._status));

    QGraphicsScene scene;
    scene.setSceneRect(QRectF(QPointF(0, 0), _accountIconSize));

    // Draw circle
    scene.addEllipse(scene.sceneRect(), QPen(Qt::NoPen), QBrush(_accountIconColor));

    // Add drive icon
    if (_item.accountInfo()._color.isValid()) {
        QGraphicsColorizeEffect *effect = new QGraphicsColorizeEffect;
        effect->setColor(_item.accountInfo()._color);
        effect->setStrength(1);
        accountItem->setGraphicsEffect(effect);
    }
    scene.addItem(accountItem);
    accountItem->setPos(QPointF((scene.width() - _driveIconSize.width()) / 2,
                                (scene.height() - _driveIconSize.height()) / 2));

    // Add status icon
    scene.addItem(statusItem);
    statusItem->setPos(QPointF(scene.width() - _statusIconSize.width(),
                               scene.height() - _statusIconSize.height()));
    qreal statusItemScale = _statusIconSize.width() / statusItem->boundingRect().width();
    statusItem->setScale(statusItemScale);

    Q_CHECK_PTR(qApp->primaryScreen());
    qreal ratio = qApp->primaryScreen()->devicePixelRatio();
    QPixmap pixmap(QSize(scene.width() * ratio, scene.height() * ratio));
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    scene.render(&painter);

    QIcon iconWithStatus;
    iconWithStatus.addPixmap(pixmap);

    return iconWithStatus;
}

void AccountItemWidget::setAccountIcon()
{
    if (_accountIconSize != QSize() && _accountIconColor != QColor()
            && _driveIconSize != QSize() && _statusIconSize != QSize()) {
        QIcon accountIconWithStatus = getIconWithStatus();
        _accountIconLabel->setPixmap(accountIconWithStatus.pixmap(_accountIconSize));
    }
}

void AccountItemWidget::onAccountIconSizeChanged()
{
    setAccountIcon();
}

void AccountItemWidget::onAccountIconColorChanged()
{
    setAccountIcon();
}

void AccountItemWidget::onDriveIconSizeChanged()
{
    setAccountIcon();
}

void AccountItemWidget::onStatusIconSizeChanged()
{
    setAccountIcon();
}

void AccountItemWidget::onMenuButtonClicked()
{
    MenuWidget *menu = new MenuWidget(MenuWidget::Menu, this);

    if (_item.accountInfo()._status == OCC::SyncResult::Problem) {
        QWidgetAction *seeSyncErrorsAction = new QWidgetAction(this);
        MenuItemWidget *seeSyncErrorsMenuItemWidget = new MenuItemWidget(tr("See synchronization errors"));
        seeSyncErrorsMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/warning.svg");
        seeSyncErrorsAction->setDefaultWidget(seeSyncErrorsMenuItemWidget);
        connect(seeSyncErrorsAction, &QWidgetAction::triggered, this, &AccountItemWidget::onSeeSyncErrorsTriggered);
        menu->addAction(seeSyncErrorsAction);
    }

    if (OCC::Utility::getSyncActionAvailable(_item.accountInfo()._paused, _item.accountInfo()._status, 0)) {
        QWidgetAction *syncAction = new QWidgetAction(this);
        MenuItemWidget *syncMenuItemWidget = new MenuItemWidget(tr("Force synchronization"));
        syncMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/sync.svg");
        syncAction->setDefaultWidget(syncMenuItemWidget);
        connect(syncAction, &QWidgetAction::triggered, this, &AccountItemWidget::onSyncTriggered);
        menu->addAction(syncAction);
    }

    if (OCC::Utility::getPauseActionAvailable(_item.accountInfo()._paused, _item.accountInfo()._status, 0)) {
        QWidgetAction *pauseAction = new QWidgetAction(this);
        MenuItemWidget *pauseMenuItemWidget = new MenuItemWidget(tr("Pause synchronization"));
        pauseMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/pause.svg");
        pauseAction->setDefaultWidget(pauseMenuItemWidget);
        connect(pauseAction, &QWidgetAction::triggered, this, &AccountItemWidget::onPauseTriggered);
        menu->addAction(pauseAction);
    }

    if (OCC::Utility::getResumeActionAvailable(_item.accountInfo()._paused, _item.accountInfo()._status, 0)) {
        QWidgetAction *resumeAction = new QWidgetAction(this);
        MenuItemWidget *resumeMenuItemWidget = new MenuItemWidget(tr("Resume synchronization"));
        resumeMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/start.svg");
        resumeAction->setDefaultWidget(resumeMenuItemWidget);
        connect(resumeAction, &QWidgetAction::triggered, this, &AccountItemWidget::onResumeTriggered);
        menu->addAction(resumeAction);
    }

    QWidgetAction *parametersAction = new QWidgetAction(this);
    MenuItemWidget *parametersMenuItemWidget = new MenuItemWidget(tr("Settings"));
    parametersMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/parameters.svg");
    parametersAction->setDefaultWidget(parametersMenuItemWidget);
    connect(parametersAction, &QWidgetAction::triggered, this, &AccountItemWidget::onParametersTriggered);
    menu->addAction(parametersAction);

    QWidgetAction *manageOfferAction = new QWidgetAction(this);
    MenuItemWidget *manageOfferMenuItemWidget = new MenuItemWidget(tr("Manage my offer in Infomaniak manager"));
    manageOfferMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/webview.svg");
    manageOfferAction->setDefaultWidget(manageOfferMenuItemWidget);
    connect(manageOfferAction, &QWidgetAction::triggered, this, &AccountItemWidget::onManageOfferTriggered);
    menu->addAction(manageOfferAction);

    menu->addSeparator();

    QWidgetAction *signOutAction = new QWidgetAction(this);
    MenuItemWidget *signOutMenuItemWidget = new MenuItemWidget(tr("Sign out"));
    signOutMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/logout.svg");
    signOutAction->setDefaultWidget(signOutMenuItemWidget);
    connect(signOutAction, &QWidgetAction::triggered, this, &AccountItemWidget::onRemoveTriggered);
    menu->addAction(signOutAction);

    menu->exec(QWidget::mapToGlobal(_menuButton->geometry().center()));
}

void AccountItemWidget::onSeeSyncErrorsTriggered()
{

}

void AccountItemWidget::onSyncTriggered()
{
    emit runSync(_item.getId());
}

void AccountItemWidget::onPauseTriggered()
{
    emit pauseSync(_item.getId());
}

void AccountItemWidget::onResumeTriggered()
{
    emit resumeSync(_item.getId());
}

void AccountItemWidget::onParametersTriggered()
{
    emit displayDriveParameters(_item.getId());
}

void AccountItemWidget::onManageOfferTriggered()
{
    emit manageOffer(_item.getId());
}

void AccountItemWidget::onRemoveTriggered()
{
    emit remove(_item.getId());
}

}
