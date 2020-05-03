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
#include <QWidget>

namespace KDC {

class PreferencesWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor action_color READ actionColor WRITE setActionColor)

public:
    explicit PreferencesWidget(QWidget *parent = nullptr);

signals:
    void actionColorChanged();

private:
    QColor _actionColor;
    QLabel *_filesToExcludeIconLabel;
    QLabel *_proxyServerIconLabel;
    QLabel *_bandwidthIconLabel;

    inline QColor actionColor() const { return _actionColor; }
    inline void setActionColor(const QColor& color) {
        _actionColor = color;
        emit actionColorChanged();
    }

private slots:
    void onActionColorChanged();
    void onFolderConfirmationCheckBoxStateChanged(int state);
    void onDarkThemeCheckBoxStateChanged(int state);
    void onMonochromeCheckBoxStateChanged(int state);
    void onLaunchAtStartupCheckBoxStateChanged(int state);
    void onFilesToExcludeWidgetClicked();
    void onProxyServerWidgetClicked();
    void onBandwidthWidgetClicked();
};

}

