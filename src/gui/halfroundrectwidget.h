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

#include <QColor>
#include <QHBoxLayout>
#include <QPaintEvent>
#include <QWidget>

namespace KDC {

class HalfRoundRectWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor bottom_corners_color READ bottomCornersColor WRITE setBottomCornersColor)

public:
    explicit HalfRoundRectWidget(QWidget *parent = nullptr);

    void setContentsMargins(int left, int top, int right, int bottom);
    void setSpacing(int spacing);
    void addWidget(QWidget *widget, int stretch = 0, Qt::Alignment alignment = Qt::Alignment());
    void addStretch(int stretch = 0);
    void addSpacing(int size);

private:
    QColor _bottomCornersColor;
    QHBoxLayout *_hboxLayout;

    void paintEvent(QPaintEvent *event) override;

    inline QColor bottomCornersColor() const { return _bottomCornersColor; }
    inline void setBottomCornersColor(const QColor &value) { _bottomCornersColor = value; }
};

}
