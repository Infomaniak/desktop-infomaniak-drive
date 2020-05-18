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
#include "customradiobutton.h"
#include "customcheckbox.h"

#include <QBoxLayout>
#include <QLineEdit>
#include <QNetworkProxy>
#include <QPushButton>

namespace KDC {

class ProxyServerDialog : public CustomDialog
{
    Q_OBJECT

public:
    explicit ProxyServerDialog(QWidget *parent = nullptr);

    void initUI();

private:
    int _proxyType;
    QNetworkProxy _proxy;
    CustomRadioButton *_noProxyButton;
    CustomRadioButton *_systemProxyButton;
    CustomRadioButton *_manualProxyButton;
    QVBoxLayout *_manualProxyVBox;
    QLineEdit *_portLineEdit;
    QLineEdit *_addressLineEdit;
    CustomCheckBox *_authenticationCheckBox;
    QHBoxLayout *_authenticationHBox;
    QLineEdit *_loginLineEdit;
    QLineEdit *_pwdLineEdit;
    QPushButton *_saveButton;
    bool _needToSave;

    void updateWidgets();

private slots:
    void onExit();
    void onSaveButtonTriggered(bool checked = false);
    void onNoProxyButtonToggled(bool checked);
};

}

