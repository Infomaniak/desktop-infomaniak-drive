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

#include "proxyserverdialog.h"
#include "configfile.h"

#include <QComboBox>
#include <QIntValidator>
#include <QLabel>
#include <QMessageBox>
#include <QRadioButton>

namespace KDC {

static const int boxHMargin= 40;
static const int boxHSpacing = 10;
static const int titleBoxVMargin = 14;
static const int proxyBoxVMargin = 15;
static const int proxyBoxSpacing = 15;
static const int manualProxyBoxHMargin = 60;
static const int manualProxyBoxVMargin = 15;
static const int portLineEditSize = 80;
static const int defaultProxyPort = 8080;

ProxyServerDialog::ProxyServerDialog(QWidget *parent)
    : CustomDialog(parent)
    , _proxyType(QNetworkProxy::NoProxy)
    , _proxy(QNetworkProxy())
    , _noProxyButton(nullptr)
    , _systemProxyButton(nullptr)
    , _manualProxyButton(nullptr)
    , _manualProxyVBox(nullptr)
    , _portLineEdit(nullptr)
    , _addressLineEdit(nullptr)
    , _authenticationCheckBox(nullptr)
    , _authenticationHBox(nullptr)
    , _loginLineEdit(nullptr)
    , _pwdLineEdit(nullptr)
    , _saveButton(nullptr)
    , _needToSave(false)
{
    initUI();
}

void ProxyServerDialog::initUI()
{
    QVBoxLayout *mainLayout = this->mainLayout();

    OCC::ConfigFile cfg;

    if (cfg.exists()) {
        _proxyType = cfg.proxyType();
        if (!cfg.proxyHostName().isEmpty()) {
            _proxy.setHostName(cfg.proxyHostName());
            _proxy.setPort(cfg.proxyPort());
            if (cfg.proxyNeedsAuth()) {
                _proxy.setUser(cfg.proxyUser());
                _proxy.setPassword(cfg.proxyPassword());
            }
        }
    }
    else {
        _proxyType = QNetworkProxy::NoProxy;
    }

    // Title
    QLabel *titleLabel = new QLabel(this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setContentsMargins(boxHMargin, 0, boxHMargin, titleBoxVMargin);
    titleLabel->setText(tr("Proxy server"));
    mainLayout->addWidget(titleLabel);

    // Proxy
    QVBoxLayout *proxyVBox = new QVBoxLayout();
    proxyVBox->setContentsMargins(boxHMargin, 0, boxHMargin, proxyBoxVMargin);
    proxyVBox->setSpacing(proxyBoxSpacing);
    mainLayout->addLayout(proxyVBox);

    _noProxyButton = new CustomRadioButton(this);
    _noProxyButton->setText(tr("No proxy server"));
    proxyVBox->addWidget(_noProxyButton);

    _systemProxyButton = new CustomRadioButton(this);
    _systemProxyButton->setText(tr("Use system parameters"));
    proxyVBox->addWidget(_systemProxyButton);

    _manualProxyButton = new CustomRadioButton(this);
    _manualProxyButton->setText(tr("Indicate a proxy manually"));
    proxyVBox->addWidget(_manualProxyButton);

    // Manual proxy
    _manualProxyVBox = new QVBoxLayout();
    _manualProxyVBox->setContentsMargins(manualProxyBoxHMargin, 0, boxHMargin, manualProxyBoxVMargin);
    _manualProxyVBox->setSpacing(proxyBoxSpacing);
    mainLayout->addLayout(_manualProxyVBox);

    QComboBox *proxyTypeComboBox = new QComboBox(this);
    proxyTypeComboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    proxyTypeComboBox->addItem(tr("Socks 5 Proxy"));
    proxyTypeComboBox->addItem(tr("Http Proxy"));
    proxyTypeComboBox->addItem(tr("Http caching Proxy"));
    proxyTypeComboBox->addItem(tr("Ftp caching Proxy"));
    _manualProxyVBox->addWidget(proxyTypeComboBox);

    QHBoxLayout *manualProxyPortHBox = new QHBoxLayout();
    manualProxyPortHBox->setContentsMargins(0, 0, 0, 0);
    manualProxyPortHBox->setSpacing(proxyBoxSpacing);
    _manualProxyVBox->addLayout(manualProxyPortHBox);

    QLabel *portLabel = new QLabel(this);
    portLabel->setText(tr("Port"));
    manualProxyPortHBox->addWidget(portLabel);

    _portLineEdit = new QLineEdit(this);
    _portLineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    _portLineEdit->setValidator(new QIntValidator(0, 65535, this));
    _portLineEdit->setMinimumWidth(portLineEditSize);
    _portLineEdit->setMaximumWidth(portLineEditSize);
    manualProxyPortHBox->addWidget(_portLineEdit);
    manualProxyPortHBox->addStretch();

    _addressLineEdit = new QLineEdit(this);
    _addressLineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    _manualProxyVBox->addWidget(_addressLineEdit);

    _authenticationCheckBox = new CustomCheckBox(this);
    _authenticationCheckBox->setText(tr("Authentication needed"));
    _manualProxyVBox->addWidget(_authenticationCheckBox);

    _authenticationHBox = new QHBoxLayout();
    _authenticationHBox->setContentsMargins(0, 0, 0, 0);
    _authenticationHBox->setSpacing(proxyBoxSpacing);
    _manualProxyVBox->addLayout(_authenticationHBox);

    _loginLineEdit = new QLineEdit(this);
    _loginLineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    _authenticationHBox->addWidget(_loginLineEdit);

    _pwdLineEdit = new QLineEdit(this);
    _pwdLineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    _pwdLineEdit->setEchoMode(QLineEdit::Password);
    _authenticationHBox->addWidget(_pwdLineEdit);

    mainLayout->addStretch();

    // Add dialog buttons
    QHBoxLayout *buttonsHBox = new QHBoxLayout();
    buttonsHBox->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    buttonsHBox->setSpacing(boxHSpacing);
    mainLayout->addLayout(buttonsHBox);

    _saveButton = new QPushButton(this);
    _saveButton->setObjectName("defaultbutton");
    _saveButton->setFlat(true);
    _saveButton->setText(tr("SAVE"));
    _saveButton->setEnabled(false);
    buttonsHBox->addWidget(_saveButton);

    QPushButton *cancelButton = new QPushButton(this);
    cancelButton->setFlat(true);
    cancelButton->setText(tr("CANCEL"));
    buttonsHBox->addWidget(cancelButton);
    buttonsHBox->addStretch();

    connect(_saveButton, &QPushButton::clicked, this, &ProxyServerDialog::onSaveButtonTriggered);
    connect(cancelButton, &QPushButton::clicked, this, &ProxyServerDialog::onExit);
    connect(this, &CustomDialog::exit, this, &ProxyServerDialog::onExit);
    connect(_noProxyButton, &CustomRadioButton::toggled, this, &ProxyServerDialog::onNoProxyButtonToggled);
}

void ProxyServerDialog::updateWidgets()
{
    bool manualProxy = false;
    if (_proxyType == QNetworkProxy::NoProxy) {
        _noProxyButton->setChecked(true);
    }
    else if (_proxyType == QNetworkProxy::DefaultProxy) {
        _systemProxyButton->setChecked(true);
    }
    else {
        _manualProxyButton->setChecked(true);
        manualProxy = true;


    }

    _manualProxyVBox->setEnabled(manualProxy);
    _portLineEdit->setText(QString::number(manualProxy ? _proxy.port() : defaultProxyPort));
    _addressLineEdit->setText(manualProxy ? _proxy.hostName() : QString());

    bool authentication = manualProxy && _proxy.user() != QString();
    _authenticationCheckBox->setEnabled(authentication);
    _authenticationHBox->setEnabled(authentication);
    _loginLineEdit->setText(authentication ? _proxy.user() : QString());
    _pwdLineEdit->setText(authentication ? _proxy.password() : QString());
}

void ProxyServerDialog::onExit()
{
    if (_needToSave) {
        QMessageBox msgBox(QMessageBox::Question, QString(),
                           tr("Do you want to save your modifications?"),
                           QMessageBox::Yes | QMessageBox::No, this);
        msgBox.setWindowModality(Qt::WindowModal);
        msgBox.setDefaultButton(QMessageBox::Yes);
        if (msgBox.exec() == QMessageBox::Yes) {
            onSaveButtonTriggered();
        }
        else {
            reject();
        }
    }
    else {
        reject();
    }
}

void ProxyServerDialog::onSaveButtonTriggered(bool checked)
{
    Q_UNUSED(checked)



}

void ProxyServerDialog::onNoProxyButtonToggled(bool checked)
{
    if (checked) {
        _proxyType = QNetworkProxy::NoProxy;
    }
    updateWidgets();
}

}
