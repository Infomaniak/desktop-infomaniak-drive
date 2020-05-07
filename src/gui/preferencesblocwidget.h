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
#include <QLabel>
#include <QWidget>

namespace KDC {

class PreferencesBlocWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor background_color READ backgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(QColor action_color READ actionColor WRITE setActionColor)

public:
    explicit PreferencesBlocWidget(QWidget *parent = nullptr);

    QBoxLayout *addLayout(QBoxLayout::Direction direction);
    ClickableWidget *addActionWidget(QVBoxLayout **vLayout);
    void addSeparator();

signals:
    void actionColorChanged();

private:
    QColor _backgroundColor;
    QColor _actionColor;
    QVBoxLayout *_layout;

    void paintEvent(QPaintEvent* event) override;

    inline QColor backgroundColor() const { return _backgroundColor; }
    inline void setBackgroundColor(const QColor &value) { _backgroundColor = value; }

    inline QColor actionColor() const { return _actionColor; }
    inline void setActionColor(const QColor& color) {
        _actionColor = color;
        emit actionColorChanged();
    }

private slots:
    void onActionColorChanged();
};

}

