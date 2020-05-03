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

#include "halfroundrectwidget.h"
#include "customtoolbutton.h"

#include <QLabel>
#include <QWidget>

namespace KDC {

class MainMenuBarWidget : public HalfRoundRectWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor logo_color READ logoColor WRITE setLogoColor)

public:
    explicit MainMenuBarWidget(QWidget *parent = nullptr);

signals:
    void logoColorChanged();
    void drivesButtonClicked();
    void preferencesButtonClicked();
    void openHelp();

private:
    QColor _logoColor;
    QLabel *_logoTextIconLabel;

    inline QColor logoColor() const { return _logoColor; }
    inline void setLogoColor(const QColor& color) {
        _logoColor = color;
        emit logoColorChanged();
    }

private slots:
    void onLogoColorChanged();
    void onDrivesButtonClicked(bool checked = false);
    void onPreferencesButtonClicked(bool checked = false);
    void onHelpButtonClicked(bool checked = false);
};

}

