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
    Q_PROPERTY(QColor action_icon_color READ actionIconColor WRITE setActionIconColor)
    Q_PROPERTY(QSize action_icon_size READ actionIconSize WRITE setActionIconSize)

public:
    static const int actionIconPathRole = Qt::UserRole;

    explicit CustomDialog(bool popup, QWidget *parent = nullptr);

    inline QVBoxLayout *mainLayout() const { return _layout; }

signals:
    void exit();
    void actionIconSet();

protected:
    inline QColor actionIconColor() const { return _actionIconColor; }
    inline QSize actionIconSize() const { return _actionIconSize; }

private:
    QColor _backgroundColor;
    QColor _actionIconColor;
    QSize _actionIconSize;
    QVBoxLayout *_layout;

    void paintEvent(QPaintEvent *event) override;

    inline QColor backgroundColor() const { return _backgroundColor; }
    inline void setBackgroundColor(const QColor &value) { _backgroundColor = value; }

    void setActionIconColor(const QColor &color);
    void setActionIconSize(const QSize &size);

    virtual void setActionIcon() {};

private slots:
    void onDrag(const QPoint &move);
    void onExit();
};

}
