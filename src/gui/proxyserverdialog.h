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
#include "customcombobox.h"

#include <QBoxLayout>
#include <QIntValidator>
#include <QLineEdit>
#include <QNetworkProxy>
#include <QPushButton>

namespace KDC {

class ProxyServerDialog : public CustomDialog
{
    Q_OBJECT

public:
    explicit ProxyServerDialog(QWidget *parent = nullptr);

private:
    static std::map<QNetworkProxy::ProxyType, std::pair<int, QString>> _manualProxyMap;

    class PortValidator : public QIntValidator
    {
    public:
        PortValidator(QObject *parent = nullptr)
            : QIntValidator(0, 65535, parent)
        {
        }

    private:
        QValidator::State validate(QString &input, int &pos) const override
        {
            QValidator::State state = QIntValidator::validate(input, pos);
            return state == QValidator::Intermediate ? QValidator::Invalid : state;
        }
    };

    QNetworkProxy::ProxyType _proxyType;
    QNetworkProxy _proxy;
    bool _proxyNeedsAuth;
    CustomRadioButton *_noProxyButton;
    CustomRadioButton *_systemProxyButton;
    CustomRadioButton *_manualProxyButton;
    QWidget *_manualProxyWidget;
    CustomComboBox *_proxyTypeComboBox;
    QLineEdit *_portLineEdit;
    QLineEdit *_addressLineEdit;
    CustomCheckBox *_authenticationCheckBox;
    QWidget *_authenticationWidget;
    QLineEdit *_loginLineEdit;
    QLineEdit *_pwdLineEdit;
    QPushButton *_saveButton;
    bool _needToSave;
    PortValidator *_portValidator;

    void initUI();
    void updateUI();
    void setNeedToSave(bool value);
    bool isSaveEnabled();
    void resetManualProxy();
    void resetAuthentication();

private slots:
    void onExit();
    void onSaveButtonTriggered(bool checked = false);
    void onNoProxyButtonClicked(bool checked = false);
    void onSystemProxyButtonClicked(bool checked = false);
    void onManualProxyButtonClicked(bool checked = false);
    void onProxyTypeComboBoxActivated(int index);
    void onPortTextEdited(const QString &text);
    void onAddressTextEdited(const QString &text);
    void onAuthenticationCheckBoxClicked(bool checked = false);
    void onLoginTextEdited(const QString &text);
    void onPwdTextEdited(const QString &text);
};

}

Q_DECLARE_METATYPE(QNetworkProxy::ProxyType)


