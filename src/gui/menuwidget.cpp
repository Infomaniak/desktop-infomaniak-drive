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

#include "menuwidget.h"
#include "guiutility.h"

#include <QAction>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QTimer>

namespace KDC {

const std::string MenuWidget::actionTypeProperty = "actionType";

static const int contentMargin = 10;
static const int cornerRadius = 5;
static const int shadowBlurRadius = 10;
static const int menuOffsetX = -30;
static const int menuOffsetY = 10;

MenuWidget::MenuWidget(Type type, QWidget *parent)
    : QMenu(parent)
    , _type(type)
    , _backgroundColor(QColor())
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::X11BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground);

    setContentsMargins(contentMargin, contentMargin, contentMargin, contentMargin);

    // Shadow
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(this);
    effect->setBlurRadius(shadowBlurRadius);
    effect->setOffset(0);
    setGraphicsEffect(effect);

    connect(this, &MenuWidget::aboutToShow, this, &MenuWidget::onAboutToShow);
}

MenuWidget::MenuWidget(Type type, const QString &title, QWidget *parent)
    : MenuWidget(type, parent)
{
    setTitle(title);
}

void MenuWidget::paintEvent(QPaintEvent *event)
{
    QRect intRect = rect().marginsRemoved(QMargins(contentMargin, contentMargin, contentMargin - 1, contentMargin - 1));

    QPainterPath painterPath;
    painterPath.addRoundedRect(intRect, cornerRadius, cornerRadius);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(backgroundColor());
    painter.setPen(Qt::NoPen);
    painter.drawPath(painterPath);

    QMenu::paintEvent(event);
}

void MenuWidget::onAboutToShow()
{
    QPoint position;
    switch (_type) {
    case Menu:
        position = QPoint(menuOffsetX, menuOffsetY);
        break;
    case Submenu:
        position = QPoint(pos().x() < parentWidget()->pos().x() ? contentMargin - 1 : -contentMargin, 0);
        break;
    case List:
        position = QPoint(0, 0);
        break;
    }

    QTimer::singleShot(0, [this, position](){ move(pos() + position); });
}

}
