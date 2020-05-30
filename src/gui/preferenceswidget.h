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
#include <QLineEdit>
#include <QWidget>

namespace KDC {

class PreferencesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PreferencesWidget(QWidget *parent = nullptr);

signals:
    void setStyle(bool darkTheme);

private:
    QLineEdit *_folderConfirmationAmountLineEdit;
    QLabel *_debuggingFolderLabel;

private slots:
    void onFolderConfirmationSwitchClicked(bool checked = false);
    void onFolderConfirmationAmountTextEdited(const QString &text);
    void onDarkThemeSwitchClicked(bool checked = false);
    void onMonochromeSwitchClicked(bool checked = false);
    void onLaunchAtStartupSwitchClicked(bool checked = false);
    void onDebuggingWidgetClicked();
    void onFilesToExcludeWidgetClicked();
    void onProxyServerWidgetClicked();
    void onBandwidthWidgetClicked();
    void onLinkActivated(const QString &link);
};

}

