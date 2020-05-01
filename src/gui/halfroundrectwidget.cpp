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

#include "halfroundrectwidget.h"

#include <QBrush>
#include <QPainter>
#include <QPainterPath>
#include <QPoint>
#include <QRect>

namespace KDC {

static const int cornerRadius = 40;

HalfRoundRectWidget::HalfRoundRectWidget(QWidget *parent)
    : QWidget(parent)
    , _bottomCornersColor(QColor())
    , _hboxLayout(nullptr)
{
    _hboxLayout = new QHBoxLayout(this);
    setLayout(_hboxLayout);
}

void HalfRoundRectWidget::setContentsMargins(int left, int top, int right, int bottom)
{
    _hboxLayout->setContentsMargins(left, top, right, bottom);
}

void HalfRoundRectWidget::setSpacing(int spacing)
{
    _hboxLayout->setSpacing(spacing);
}

void HalfRoundRectWidget::addWidget(QWidget *widget, int stretch, Qt::Alignment alignment)
{
    _hboxLayout->addWidget(widget, stretch, alignment);
}

void HalfRoundRectWidget::addStretch(int stretch)
{
    _hboxLayout->addStretch(stretch);
}

void HalfRoundRectWidget::addSpacing(int size)
{
    _hboxLayout->addSpacing(size);
}

void HalfRoundRectWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPointF rectangleLeftBottom(0, rect().height());
    QPointF rectangleRightBottom(rect().width(), rect().height());

    QPainterPath painterPath1(rectangleLeftBottom);
    painterPath1.arcTo(QRect(0, rect().height() - cornerRadius, cornerRadius, cornerRadius), 180, 90);
    painterPath1.moveTo(rectangleLeftBottom);

    QPainterPath painterPath2(rectangleRightBottom);
    painterPath2.arcTo(QRect(rect().width() - cornerRadius, rect().height() - cornerRadius, cornerRadius, cornerRadius), 270, 90);
    painterPath2.moveTo(rectangleRightBottom);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(bottomCornersColor());
    painter.setPen(Qt::NoPen);
    painter.drawPath(painterPath1);
    painter.drawPath(painterPath2);
}

}
