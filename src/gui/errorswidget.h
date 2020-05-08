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

#pragma once

#include "clickablewidget.h"

#include <QColor>
#include <QLabel>
#include <QSize>

namespace KDC {

class ErrorsWidget : public ClickableWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor background_color READ backgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(QColor warning_icon_color READ warningIconColor WRITE setWarningIconColor)
    Q_PROPERTY(QSize warning_icon_size READ warningIconSize WRITE setWarningIconSize)

public:
    explicit ErrorsWidget(QWidget *parent = nullptr);

signals:
    void warningIconSizeChanged();
    void warningIconColorChanged();
    void displayErrors();

private:
    QColor _backgroundColor;
    QSize _warningIconSize;
    QColor _warningIconColor;
    QLabel *_warningIconLabel;

    inline QColor backgroundColor() const { return _backgroundColor; }
    inline void setBackgroundColor(QColor color) { _backgroundColor = color; }

    inline QSize warningIconSize() const { return _warningIconSize; }
    inline void setWarningIconSize(QSize size) {
        _warningIconSize = size;
        emit warningIconSizeChanged();
    }

    inline QColor warningIconColor() const { return _warningIconColor; }
    inline void setWarningIconColor(QColor color) {
        _warningIconColor = color;
        emit warningIconColorChanged();
    }

    void setWarningIcon();

private slots:
    void onWarningIconSizeChanged();
    void onWarningIconColorChanged();
    void onClick();
};

}

