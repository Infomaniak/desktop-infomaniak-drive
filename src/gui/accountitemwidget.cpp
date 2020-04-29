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
#include "guiutility.h"

#include <QGraphicsColorizeEffect>
#include <QGraphicsScene>
#include <QGraphicsSvgItem>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>

namespace KDC {

static const int cornerRadius = 10;
static const int hMargin= 20;
static const int vTopMargin = 0;
static const int vBottomMargin = 10;
static const int boxHMargin= 15;
static const int boxVMargin = 5;
static const int boxSpacing = 15;
static const int textBoxHMargin= 0;
static const int textBoxVMargin = 10;
static const int driveNameMaxSize = 50;

AccountItemWidget::AccountItemWidget(const QString &accountId, QWidget *parent)
    : QWidget(parent)
    , _item(accountId)
    , _backgroundColor(QColor())
    , _backgroundColorSelection(QColor())
    , _accountIconLabel(nullptr)
{
    setContentsMargins(hMargin, vTopMargin, hMargin, vBottomMargin);

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

    // Draw background
    QPainterPath painterPath1;
    painterPath1.addRect(rect());

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(backgroundColor());
    painter.drawPath(painterPath1);

    // Draw round rectangle
    QPainterPath painterPath2;
    painterPath2.addRoundedRect(rect().marginsRemoved(QMargins(hMargin, vTopMargin, hMargin, vBottomMargin)),
                                cornerRadius, cornerRadius);

    painter.setBrush(backgroundColorSelection());
    painter.drawPath(painterPath2);
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

}

}
