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

#include <QBoxLayout>
#include <QColor>
#include <QLabel>
#include <QScrollArea>
#include <QSize>
#include <QWidget>

namespace KDC {

class PreferencesBlocWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor background_color READ backgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(QColor action_icon_color READ actionIconColor WRITE setActionIconColor)
    Q_PROPERTY(QSize action_icon_size READ actionIconSize WRITE setActionIconSize)

public:
    explicit PreferencesBlocWidget(QWidget *parent = nullptr);

    QBoxLayout *addLayout(QBoxLayout::Direction direction, bool noMargins = false);
    ClickableWidget *addActionWidget(QVBoxLayout **vLayout, bool noMargins = false);
    QFrame *addSeparator();

signals:
    void actionIconColorChanged();
    void actionIconSizeChanged();

private:
    QColor _backgroundColor;
    QColor _actionIconColor;
    QSize _actionIconSize;
    QVBoxLayout *_layout;

    void paintEvent(QPaintEvent* event) override;

    inline QColor backgroundColor() const { return _backgroundColor; }
    inline void setBackgroundColor(const QColor &value) { _backgroundColor = value; }

    inline QColor actionIconColor() const { return _actionIconColor; }
    inline void setActionIconColor(const QColor& color) {
        _actionIconColor = color;
        emit actionIconColorChanged();
    }

    inline QSize actionIconSize() const { return _actionIconSize; }
    inline void setActionIconSize(const QSize& size) {
        _actionIconSize = size;
        emit actionIconSizeChanged();
    }

    void setActionIcon();

private slots:
    void onActionIconColorChanged();
    void onActionIconSizeChanged();
};

}

