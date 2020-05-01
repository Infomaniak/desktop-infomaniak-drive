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
#include <QLabel>
#include <QPushButton>
#include <QSize>

namespace KDC {

class AddDriveWidget : public QPushButton
{
    Q_OBJECT

    Q_PROPERTY(QSize add_icon_size READ addIconSize WRITE setAddIconSize)
    Q_PROPERTY(QColor add_icon_color READ addIconColor WRITE setAddIconColor)

public:
    explicit AddDriveWidget(QWidget *parent = nullptr);
    QSize sizeHint() const override;

signals:
    void addIconSizeChanged();
    void addIconColorChanged();
    void addDrive();

private:
    QSize _addIconSize;
    QColor _addIconColor;
    QLabel *_addIconLabel;
    QLabel *_addTextLabel;

    inline QSize addIconSize() const { return _addIconSize; }
    inline void setAddIconSize(QSize size) {
        _addIconSize = size;
        emit addIconSizeChanged();
    }

    inline QColor addIconColor() const { return _addIconColor; }
    inline void setAddIconColor(QColor color) {
        _addIconColor = color;
        emit addIconColorChanged();
    }

    void setAddIcon();

private slots:
    void onAddIconSizeChanged();
    void onAddIconColorChanged();
    void onClick(bool checked);
};

}
