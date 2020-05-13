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

#include "actionwidget.h"
#include "guiutility.h"

#include <QHBoxLayout>
#include <QPainterPath>
#include <QPainter>

namespace KDC {

static const int boxHMargin= 15;
static const int boxVMargin = 5;
static const int boxSpacing = 10;

ActionWidget::ActionWidget(const QString &path, const QString &text, QWidget *parent)
    : ClickableWidget(parent)
    , _leftIconPath(path)
    , _text(text)
    , _backgroundColor(QColor())
    , _leftIconColor(QColor())
    , _leftIconSize(QSize())
    , _actionIconColor(QColor())
    , _actionIconSize(QSize())
    , _leftIconLabel(nullptr)
    , _actionIconLabel(nullptr)
{
    setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *hbox = new QHBoxLayout(this);
    hbox->setContentsMargins(boxHMargin, boxVMargin, boxHMargin, boxVMargin);
    hbox->setSpacing(boxSpacing);
    setLayout(hbox);

    _leftIconLabel = new QLabel(this);
    hbox->addWidget(_leftIconLabel);

    QLabel *leftTextLabel = new QLabel(_text, this);
    leftTextLabel->setObjectName("textLabel");
    hbox->addWidget(leftTextLabel);
    hbox->addStretch();

    _actionIconLabel = new QLabel(this);
    hbox->addWidget(_actionIconLabel);

    connect(this, &ActionWidget::leftIconSizeChanged, this, &ActionWidget::onLeftIconSizeChanged);
    connect(this, &ActionWidget::leftIconColorChanged, this, &ActionWidget::onLeftIconColorChanged);
    connect(this, &ActionWidget::actionIconColorChanged, this, &ActionWidget::onActionIconColorChanged);
    connect(this, &ActionWidget::actionIconSizeChanged, this, &ActionWidget::onActionIconSizeChanged);
}

void ActionWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    // Draw round rectangle
    QPainterPath painterPath;
    painterPath.addRoundedRect(rect(), rect().height() / 2, rect().height() / 2);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(backgroundColor());
    painter.drawPath(painterPath);
}

void ActionWidget::setLeftIcon()
{
    if (_leftIconLabel && _leftIconSize != QSize() && _leftIconColor != QColor()) {
        _leftIconLabel->setPixmap(OCC::Utility::getIconWithColor(_leftIconPath, _leftIconColor)
                                     .pixmap(_leftIconSize));
    }
}

void ActionWidget::setActionIcon()
{
    if (_actionIconLabel && _actionIconSize != QSize() && _actionIconColor != QColor()) {
        _actionIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/chevron-right.svg", _actionIconColor)
                                    .pixmap(_actionIconSize));
    }
}

void ActionWidget::onLeftIconSizeChanged()
{
    setLeftIcon();
}

void ActionWidget::onLeftIconColorChanged()
{
    setLeftIcon();
}

void ActionWidget::onActionIconColorChanged()
{
    setActionIcon();
}

void ActionWidget::onActionIconSizeChanged()
{
    setActionIcon();
}

}
