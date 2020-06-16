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

#include <QAction>
#include <QColor>
#include <QMenu>
#include <QPaintEvent>
#include <QPoint>
#include <QString>

namespace KDC {

class MenuWidget : public QMenu
{
    Q_OBJECT

    Q_PROPERTY(QColor background_color READ backgroundColor WRITE setBackgroundColor)

public:
    enum Type {
        Menu = 0,
        Submenu,
        List
    };

    static const std::string actionTypeProperty;

    MenuWidget(Type type, QWidget *parent = nullptr);
    MenuWidget(Type type, const QString &title, QWidget *parent = nullptr);

private:
    Type _type;
    QColor _backgroundColor;
    bool _painted;

    void paintEvent(QPaintEvent *event) override;

    inline QColor backgroundColor() const { return _backgroundColor; }
    inline void setBackgroundColor(const QColor &color) { _backgroundColor = color; }

private slots:
    void onAboutToShow();
};

}
