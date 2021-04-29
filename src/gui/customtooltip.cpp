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

#include "customtooltip.h"
#include "guiutility.h"

#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QLoggingCategory>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>
#include <QTimer>

namespace KDC {

static const int triangleHeight = 6;
static const int triangleWidth  = 10;
static const int contentMargin = 10;
static const int cornerRadius = 5;
static const int shadowBlurRadius = 10;
static const int boxMargin = 10;
static const int offsetX = 0;
static const int offsetY = 10;

Q_LOGGING_CATEGORY(lcCustomToolTip, "gui.customtooltip", QtInfoMsg)

CustomToolTip::CustomToolTip(const QString &text, const QPoint &position, int toolTipDuration, QWidget *parent)
    : QDialog(parent)
    , _cursorPosition(position)
{
    setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::X11BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground);

    setContentsMargins(contentMargin, contentMargin, contentMargin, contentMargin);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(boxMargin, boxMargin, boxMargin, boxMargin);
    layout->setSpacing(0);

    QLabel *tooltipLabel = new QLabel(this);
    tooltipLabel->setObjectName("tooltipLabel");
    tooltipLabel->setText(text);
    layout->addWidget(tooltipLabel);

    setLayout(layout);

    // Shadow
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(this);
    effect->setBlurRadius(shadowBlurRadius);
    effect->setOffset(0);
    setGraphicsEffect(effect);

    if (toolTipDuration) {
        QTimer::singleShot(toolTipDuration, this, &CustomToolTip::close);
    }
}

void CustomToolTip::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    // Update shadow color
    QGraphicsDropShadowEffect *effect = qobject_cast<QGraphicsDropShadowEffect *>(graphicsEffect());
    if (effect) {
        effect->setColor(OCC::Utility::getShadowColor(true));
    }

    // Calculate position
    QPoint tooltipPosition = QPoint(_cursorPosition.x() - rect().width() / 2.0 + offsetX,
                                    _cursorPosition.y() + offsetY);

    // Triangle points
    QPointF trianglePoint1 = QPoint((rect().width() - triangleWidth) / 2.0, triangleHeight);
    QPointF trianglePoint2 = QPoint(rect().width() / 2.0, 1);
    QPointF trianglePoint3 = QPoint((rect().width() + triangleWidth) / 2.0, triangleHeight);

    // Border
    int cornerDiameter = 2 * cornerRadius;
    QRect intRect = rect().marginsRemoved(QMargins(triangleHeight, triangleHeight, triangleHeight - 1, triangleHeight - 1));
    QPainterPath painterPath;
    painterPath.moveTo(trianglePoint3);
    painterPath.lineTo(trianglePoint2);
    painterPath.lineTo(trianglePoint1);
    painterPath.arcTo(QRect(intRect.topLeft(), QSize(cornerDiameter, cornerDiameter)), 90, 90);
    painterPath.arcTo(QRect(intRect.bottomLeft() - QPoint(0, cornerDiameter), QSize(cornerDiameter, cornerDiameter)), 180, 90);
    painterPath.arcTo(QRect(intRect.bottomRight() - QPoint(cornerDiameter, cornerDiameter), QSize(cornerDiameter, cornerDiameter)), 270, 90);
    painterPath.arcTo(QRect(intRect.topRight() - QPoint(cornerDiameter, 0), QSize(cornerDiameter, cornerDiameter)), 0, 90);
    painterPath.closeSubpath();

    move(tooltipPosition);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(QColor(backgroundColor()));
    painter.setPen(Qt::NoPen);
    painter.drawPath(painterPath);
}

}