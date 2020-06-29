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
#include <QDialog>
#include <QPaintEvent>
#include <QPoint>
#include <QStandardItem>
#include <QVBoxLayout>

namespace KDC {

class CustomDialog : public QDialog
{
    Q_OBJECT

    Q_PROPERTY(QColor background_color READ backgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(QColor button_icon_color READ buttonIconColor WRITE setButtonIconColor)

public:
    explicit CustomDialog(bool popup, QWidget *parent = nullptr);

    int exec() override;
    int exec(QPoint position);
    inline QVBoxLayout *mainLayout() const { return _layout; }
    void forceRedraw();
    void setBackgroundForcedColor(const QColor &value);

signals:
    void exit();
    void viewIconSet();

private:
    QColor _backgroundColor;
    QColor _buttonIconColor;
    QColor _backgroundForcedColor;
    QVBoxLayout *_layout;

    void paintEvent(QPaintEvent *event) override;

    inline QColor backgroundColor() const { return _backgroundColor; }
    inline void setBackgroundColor(const QColor &value) { _backgroundColor = value; }

    inline QColor buttonIconColor() const { return _buttonIconColor; }
    inline void setButtonIconColor(const QColor &value) {
        _buttonIconColor = value;
        setButtonIcon(value);
    }

    virtual void setButtonIcon(const QColor &value) { Q_UNUSED(value); };

private slots:
    void onDrag(const QPoint &move);
    void onExit();
};

}
