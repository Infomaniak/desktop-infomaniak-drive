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

#include "customtoolbutton.h"
#include "guiutility.h"

#include <iostream>

#include <QApplication>
#include <QCursor>
#include <QEvent>
#include <QPainter>

namespace KDC {

static int defaultToolTipDuration = 3000; // ms

CustomToolButton::CustomToolButton(QWidget *parent)
    : QToolButton(parent)
    , _withMenu(false)
    , _baseIconSize(QSize())
    , _iconPath(QString())
    , _iconColor(QColor())
    , _iconColorHover(QColor())
    , _toolTipText(QString())
    , _toolTipDuration(defaultToolTipDuration)
    , _customToolTip(nullptr)
{
    connect(this, &CustomToolButton::baseIconSizeChanged, this, &CustomToolButton::onBaseIconSizeChanged);
    connect(this, &CustomToolButton::iconColorChanged, this, &CustomToolButton::onIconColorChanged);
    connect(this, &CustomToolButton::clicked, this, &CustomToolButton::onClicked);
}

void CustomToolButton::setWithMenu(bool withMenu)
{
    _withMenu = withMenu;
    applyIconSizeAndColor(_iconColor);
}

void CustomToolButton::onIconColorChanged()
{
    if (!_iconPath.isEmpty()) {
        applyIconSizeAndColor(_iconColor);
    }
}

void CustomToolButton::onClicked(bool checked)
{
    //QApplication::sendEvent(this, new QEvent(QEvent::Leave));
}

bool CustomToolButton::event(QEvent *event)
{
    std::cout << "event: " << event->type() << std::endl;

    if (event->type() == QEvent::ToolTip) {
        if (!_toolTipText.isEmpty()) {
            if (!_customToolTip) {
                QRect widgetRect = geometry();
                QPoint position = parentWidget()->mapToGlobal((widgetRect.bottomLeft() + widgetRect.bottomRight()) / 2.0);
                _customToolTip = new CustomToolTip(_toolTipText, position, _toolTipDuration);
                _customToolTip->show();
                event->ignore();
                return true;
            }
        }
    }

    return QToolButton::event(event);
}

void CustomToolButton::enterEvent(QEvent *event)
{
    applyIconSizeAndColor(_iconColorHover);

    QToolButton::enterEvent(event);
}

void CustomToolButton::leaveEvent(QEvent *event)
{
    applyIconSizeAndColor(_iconColor);
    if (_customToolTip) {
        emit _customToolTip->close();
        _customToolTip = nullptr;
    }

    QToolButton::leaveEvent(event);
}

void CustomToolButton::applyIconSizeAndColor(const QColor &color)
{
    if (_baseIconSize != QSize()) {
        setIconSize(QSize(_withMenu ? 2 * _baseIconSize.width() : _baseIconSize.width(),
                          _baseIconSize.height()));
    }

    if (!_iconPath.isEmpty() && color.isValid()) {
        if (_withMenu) {
            setIcon(OCC::Utility::getIconMenuWithColor(_iconPath, color));
        }
        else {
            setIcon(OCC::Utility::getIconWithColor(_iconPath, color));
        }
    }
}

void CustomToolButton::onBaseIconSizeChanged()
{
    if (!_iconPath.isEmpty()) {
        applyIconSizeAndColor(_iconColor);
    }
}

}
