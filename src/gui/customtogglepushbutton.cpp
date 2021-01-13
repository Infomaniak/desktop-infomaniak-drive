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

#include "customtogglepushbutton.h"
#include "guiutility.h"

namespace KDC {

CustomTogglePushButton::CustomTogglePushButton(QWidget *parent)
    : QPushButton(parent)
    , _iconPath(QString())
    , _iconColor(QColor())
    , _iconColorChecked(QColor())
{
    setFlat(true);
    setCheckable(true);

    connect(this, &CustomTogglePushButton::iconColorChanged, this, &CustomTogglePushButton::onIconColorChanged);
    connect(this, &CustomTogglePushButton::iconColorCheckedChanged, this, &CustomTogglePushButton::onIconColorCheckedChanged);
    connect(this, &CustomTogglePushButton::toggled, this, &CustomTogglePushButton::onToggle);
}

CustomTogglePushButton::CustomTogglePushButton(const QString &text, QWidget *parent)
    : CustomTogglePushButton(parent)
{
    setText(text);
}

bool CustomTogglePushButton::event(QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress
            || event->type() == QEvent::MouseButtonDblClick
            || event->type() == QEvent::KeyPress) {
        if (isChecked()) {
            event->ignore();
            return true;
        }
    }
    return QPushButton::event(event);
}

void CustomTogglePushButton::onIconColorChanged()
{
    onToggle(isChecked());
}

void CustomTogglePushButton::onIconColorCheckedChanged()
{
    onToggle(isChecked());
}

void CustomTogglePushButton::onToggle(bool checked)
{
    if (!_iconPath.isEmpty()) {
        setIcon(OCC::Utility::getIconWithColor(_iconPath, checked ? _iconColorChecked : _iconColor));
    }
}

}
