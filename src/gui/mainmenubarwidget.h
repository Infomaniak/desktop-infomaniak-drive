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
#include "driveselectionwidget.h"
#include "progressbarwidget.h"

#include <QLabel>
#include <QWidget>

namespace KDC {

class MainMenuBarWidget : public HalfRoundRectWidget
{
    Q_OBJECT

public:
    explicit MainMenuBarWidget(QWidget *parent = nullptr);

    inline DriveSelectionWidget *driveSelectionWidget() const { return _driveSelectionWidget; }
    inline ProgressBarWidget *progressBarWidget() const { return _progressBarWidget; }

signals:
    void accountSelected(const QString &accountId);
    void addDrive();
    void preferencesButtonClicked();
    void openHelp();

private:
    DriveSelectionWidget *_driveSelectionWidget;
    ProgressBarWidget *_progressBarWidget;

private slots:
    void onAccountSelected(QString id);
    void onAddDrive();
    void onPreferencesButtonClicked(bool checked = false);
    void onHelpButtonClicked(bool checked = false);
};

}

