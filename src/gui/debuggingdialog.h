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

#include "customdialog.h"
#include "customswitch.h"
#include "customcombobox.h"

#include <QPushButton>

namespace KDC {

class DebuggingDialog : public CustomDialog
{
    Q_OBJECT

public:
    explicit DebuggingDialog(QWidget *parent = nullptr);

private:
    enum DebugLevel {
        Info = 0,
        Debug,
        Warning,
        Critical,
        Fatal
    };

    static std::map<DebugLevel, std::pair<int, QString>> _debugLevelMap;

    CustomSwitch *_recordDebuggingSwitch;
    CustomComboBox *_debugLevelComboBox;
    QPushButton *_saveButton;
    bool _needToSave;

    void initUI();
    void updateUI();
    void setNeedToSave(bool value);

private slots:
    void onExit();
    void onSaveButtonTriggered(bool checked = false);
};

}
