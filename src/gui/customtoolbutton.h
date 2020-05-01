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

#include "customtooltip.h"

#include <QColor>
#include <QPaintEvent>
#include <QToolButton>
#include <QSize>
#include <QString>

namespace KDC {

class CustomToolButton : public QToolButton
{
    Q_OBJECT

    Q_PROPERTY(QSize base_icon_size READ baseIconSize WRITE setBaseIconSize)
    Q_PROPERTY(QColor icon_color READ iconColor WRITE setIconColor)
    Q_PROPERTY(QColor icon_color_hover READ iconColorHover WRITE setIconColorHover)

public:
    explicit CustomToolButton(QWidget *parent = nullptr);

    inline bool withMenu() const { return _withMenu; };
    void setWithMenu(bool withMenu);

    inline QString iconPath() const { return _iconPath; };
    inline void setIconPath(const QString &path) { _iconPath = path; }

    inline QSize baseIconSize() const { return _baseIconSize; }
    inline void setBaseIconSize(const QSize& size) {
        _baseIconSize = size;
        emit baseIconSizeChanged();
    }

    inline QColor iconColor() const { return _iconColor; }
    inline void setIconColor(const QColor& color) {
        _iconColor = color;
        emit iconColorChanged();
    }

    inline QColor iconColorHover() const { return _iconColorHover; }
    inline void setIconColorHover(const QColor& color) { _iconColorHover = color; }

    inline void setToolTip(const QString &text) { _toolTipText = text; }
    inline void setToolTipDuration(int msec) { _toolTipDuration = msec; }

signals:
    void baseIconSizeChanged();
    void iconColorChanged();

private:
    bool _withMenu;
    QSize _baseIconSize;
    QString _iconPath;
    QColor _iconColor;
    QColor _iconColorHover;
    QString _toolTipText;
    int _toolTipDuration;
    CustomToolTip *_customToolTip;

    bool event(QEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void applyIconSizeAndColor(const QColor &color);

private slots:
    void onBaseIconSizeChanged();
    void onIconColorChanged();
    void onClicked(bool checked = false);
};

}
