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

#include "customtogglepushbutton.h"

#include <QColor>
#include <QBoxLayout>
#include <QList>
#include <QPaintEvent>
#include <QWidget>

namespace KDC {

class ButtonsBarWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor background_color READ backgroundColor WRITE setBackgroundColor)

public:
    explicit ButtonsBarWidget(QWidget *parent = nullptr);

    void insertButton(int position, CustomTogglePushButton *button);
    void selectButton(int position);
    inline int position() const { return _position; }

signals:
    void buttonToggled(int position);

private:
    int _position;
    QColor _backgroundColor;
    QHBoxLayout *_hboxLayout;
    QList<CustomTogglePushButton*> buttonsList;

    void paintEvent(QPaintEvent *event) override;

    inline QColor backgroundColor() const { return _backgroundColor; }
    inline void setBackgroundColor(const QColor& value) { _backgroundColor = value; }

private slots:
    void onToggle(bool checked);
};

}

