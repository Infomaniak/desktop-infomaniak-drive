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

#include "bottomwidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QPointF>

namespace KDC {

BottomWidget::BottomWidget(QWidget *parent)
    : QWidget(parent)
    , _backgroundColor(QColor())
{
}

void BottomWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPointF rectangleLeftTop(0, 0);
    QPointF rectangleRightTop(rect().width(), 0);

    QPainterPath painterPath(rectangleLeftTop);
    painterPath.arcTo(QRect(0, - rect().height(), 2 * rect().height(), 2 * rect().height()), 180, 90);
    painterPath.arcTo(QRect(rect().width() - 2 * rect().height(), - rect().height(), 2 * rect().height(), 2 * rect().height()), 270, 90);
    painterPath.lineTo(rectangleRightTop);
    painterPath.closeSubpath();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(backgroundColor());
    painter.setPen(Qt::NoPen);
    painter.drawPath(painterPath);
}

}
