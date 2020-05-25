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

#include "custompushbutton.h"
#include "guiutility.h"

#include <QHBoxLayout>
#include <QSizePolicy>
#include <QWidgetAction>

namespace KDC {

static const int boxHMargin= 10;
static const int boxVMargin = 5;
static const int boxSpacing = 10;

CustomPushButton::CustomPushButton(const QString &path, const QString &text, QWidget *parent)
    : QPushButton(parent)
    , _iconPath(path)
    , _text(text)
    , _iconSize(QSize())
    , _iconColor(QColor())
    , _iconLabel(nullptr)
    , _textLabel(nullptr)
{
    setContentsMargins(0, 0, 0, 0);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->setContentsMargins(boxHMargin, boxVMargin, boxHMargin, boxVMargin);
    hbox->setSpacing(boxSpacing);
    setLayout(hbox);

    _iconLabel = new QLabel(this);
    hbox->addWidget(_iconLabel);

    _textLabel = new QLabel(_text, this);
    _textLabel->setObjectName("textLabel");
    hbox->addWidget(_textLabel);
    hbox->addStretch();

    connect(this, &CustomPushButton::iconSizeChanged, this, &CustomPushButton::onIconSizeChanged);
    connect(this, &CustomPushButton::iconColorChanged, this, &CustomPushButton::onIconColorChanged);
}

QSize CustomPushButton::sizeHint() const
{
    return QSize(_iconLabel->sizeHint().width()
                 + _textLabel->sizeHint().width()
                 + boxSpacing
                 + 2 * boxHMargin,
                 QPushButton::sizeHint().height());
}

void CustomPushButton::onIconSizeChanged()
{
    setIcon();
}

void CustomPushButton::onIconColorChanged()
{
    setIcon();
}

void CustomPushButton::setIcon()
{
    if (_iconLabel && _iconSize != QSize() && _iconColor != QColor()) {
        _iconLabel->setPixmap(OCC::Utility::getIconWithColor(_iconPath, _iconColor)
                              .pixmap(_iconSize));
    }
}

}
