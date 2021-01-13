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
#include "custommessagebox.h"
#include "configfile.h"
#include "clientproxy.h"
#include "folderman.h"
#include "accountmanager.h"

#include <map>

#include <QAbstractItemView>
#include <QHostInfo>
#include <QLabel>
#include <QRadioButton>

namespace KDC {

static const int boxHMargin = 40;
static const int boxHSpacing = 10;
static const int titleBoxVMargin = 14;
static const int proxyBoxVMargin = 12;
static const int proxyBoxSpacing = 12;
static const int manualProxyBoxHMargin = 60;
static const int manualProxyBoxVMargin = 12;
static const int authenticationSpacing = 5;
static const int portLineEditSize = 80;
static const int defaultPortNumber = 8080;
static const int hostNameMaxLength = 200;

std::map<QNetworkProxy::ProxyType, std::pair<int, QString>> ProxyServerDialog::_manualProxyMap = {
    { QNetworkProxy::Socks5Proxy, { 0, QString(tr("SOCKS5 Proxy")) } },
    { QNetworkProxy::HttpProxy, { 1, QString(tr("HTTP(S) Proxy")) } }
};

ProxyServerDialog::ProxyServerDialog(QWidget *parent)
    : CustomDialog(true, parent)
    , _proxyType(QNetworkProxy::NoProxy)
    , _proxy(QNetworkProxy())
    , _proxyNeedsAuth(false)
    , _noProxyButton(nullptr)
    , _systemProxyButton(nullptr)
    , _manualProxyButton(nullptr)
    , _manualProxyWidget(nullptr)
    , _proxyTypeComboBox(nullptr)
    , _portLineEdit(nullptr)
    , _addressLineEdit(nullptr)
    , _authenticationCheckBox(nullptr)
    , _authenticationWidget(nullptr)
    , _loginLineEdit(nullptr)
    , _pwdLineEdit(nullptr)
    , _saveButton(nullptr)
    , _needToSave(false)
    , _portValidator(new PortValidator(this))
{
    initUI();

    // Load saved configuration
    OCC::ConfigFile cfg;
    if (cfg.exists()) {
        _proxyType = (QNetworkProxy::ProxyType) cfg.proxyType();
        if (_proxyType != QNetworkProxy::NoProxy && _proxyType != QNetworkProxy::DefaultProxy) {
            // Manual proxy definition
            _proxy.setHostName(cfg.proxyHostName());
            _proxy.setPort(cfg.proxyPort());
            _proxyNeedsAuth = cfg.proxyNeedsAuth();
            if (_proxyNeedsAuth) {
                _proxy.setUser(cfg.proxyUser());
                _proxy.setPassword(cfg.proxyPassword());
            }
        }
    }

    updateUI();
}

void ProxyServerDialog::initUI()
{
    QVBoxLayout *mainLayout = this->mainLayout();

    // Title
    QLabel *titleLabel = new QLabel(this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    titleLabel->setText(tr("Proxy server"));
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(titleBoxVMargin);

    // Proxy
    QVBoxLayout *proxyVBox = new QVBoxLayout();
    proxyVBox->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    proxyVBox->setSpacing(proxyBoxSpacing);
    mainLayout->addLayout(proxyVBox);
    mainLayout->addSpacing(proxyBoxVMargin);

    _noProxyButton = new CustomRadioButton(this);
    _noProxyButton->setText(tr("No proxy server"));
    _noProxyButton->setAttribute(Qt::WA_MacShowFocusRect, false);
    proxyVBox->addWidget(_noProxyButton);

    _systemProxyButton = new CustomRadioButton(this);
    _systemProxyButton->setText(tr("Use system parameters"));
    _systemProxyButton->setAttribute(Qt::WA_MacShowFocusRect, false);
    proxyVBox->addWidget(_systemProxyButton);

    _manualProxyButton = new CustomRadioButton(this);
    _manualProxyButton->setText(tr("Indicate a proxy manually"));
    _manualProxyButton->setAttribute(Qt::WA_MacShowFocusRect, false);
    proxyVBox->addWidget(_manualProxyButton);

    // Manual proxy
    _manualProxyWidget = new QWidget(this);
    _manualProxyWidget->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(_manualProxyWidget);

    QVBoxLayout *manualProxyVBox = new QVBoxLayout();
    manualProxyVBox->setContentsMargins(manualProxyBoxHMargin, 0, boxHMargin, 0);
    manualProxyVBox->setSpacing(proxyBoxSpacing);
    _manualProxyWidget->setLayout(manualProxyVBox);

    QHBoxLayout *manualProxyTypeHBox = new QHBoxLayout();
    manualProxyTypeHBox->setContentsMargins(0, 0, 0, 0);
    manualProxyVBox->addLayout(manualProxyTypeHBox);

    _proxyTypeComboBox = new CustomComboBox(this);
    _proxyTypeComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    _proxyTypeComboBox->setAttribute(Qt::WA_MacShowFocusRect, false);

    for (auto const &manualProxyMapElt : _manualProxyMap) {
        _proxyTypeComboBox->insertItem(manualProxyMapElt.second.first, manualProxyMapElt.second.second, manualProxyMapElt.first);
    }
    manualProxyTypeHBox->addWidget(_proxyTypeComboBox);
    manualProxyTypeHBox->addStretch();

    QHBoxLayout *manualProxyPortHBox = new QHBoxLayout();
    manualProxyPortHBox->setContentsMargins(0, 0, 0, 0);
    manualProxyPortHBox->setSpacing(proxyBoxSpacing);
    manualProxyVBox->addLayout(manualProxyPortHBox);

    QLabel *portLabel = new QLabel(this);
    portLabel->setObjectName("boldTextLabel");
    portLabel->setText(tr("Port"));
    manualProxyPortHBox->addWidget(portLabel);

    _portLineEdit = new QLineEdit(this);
    _portLineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    _portLineEdit->setValidator(_portValidator);
    _portLineEdit->setMinimumWidth(portLineEditSize);
    _portLineEdit->setMaximumWidth(portLineEditSize);
    manualProxyPortHBox->addWidget(_portLineEdit);
    manualProxyPortHBox->addStretch();

    _addressLineEdit = new QLineEdit(this);
    _addressLineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    _addressLineEdit->setPlaceholderText(tr("Address of the proxy server"));
    _addressLineEdit->setMaxLength(hostNameMaxLength);
    manualProxyVBox->addWidget(_addressLineEdit);
    manualProxyVBox->addSpacing(authenticationSpacing);

    _authenticationCheckBox = new CustomCheckBox(this);
    _authenticationCheckBox->setText(tr("Authentication needed"));
    manualProxyVBox->addWidget(_authenticationCheckBox);

    _authenticationWidget = new QWidget(this);
    _authenticationWidget->setContentsMargins(0, 0, 0, 0);
    manualProxyVBox->addWidget(_authenticationWidget);
    manualProxyVBox->addSpacing(manualProxyBoxVMargin);

    QHBoxLayout *authenticationHBox = new QHBoxLayout();
    authenticationHBox->setContentsMargins(0, 0, 0, 0);
    authenticationHBox->setSpacing(proxyBoxSpacing);
    _authenticationWidget->setLayout(authenticationHBox);

    _loginLineEdit = new QLineEdit(this);
    _loginLineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    _loginLineEdit->setPlaceholderText(tr("User"));
    authenticationHBox->addWidget(_loginLineEdit);

    _pwdLineEdit = new QLineEdit(this);
    _pwdLineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    _pwdLineEdit->setEchoMode(QLineEdit::Password);
    _pwdLineEdit->setPlaceholderText(tr("Password"));
    authenticationHBox->addWidget(_pwdLineEdit);

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
    cancelButton->setObjectName("nondefaultbutton");
    cancelButton->setFlat(true);
    cancelButton->setText(tr("CANCEL"));
    buttonsHBox->addWidget(cancelButton);
    buttonsHBox->addStretch();

    connect(_saveButton, &QPushButton::clicked, this, &ProxyServerDialog::onSaveButtonTriggered);
    connect(cancelButton, &QPushButton::clicked, this, &ProxyServerDialog::onExit);
    connect(this, &CustomDialog::exit, this, &ProxyServerDialog::onExit);
    connect(_noProxyButton, &CustomRadioButton::clicked, this, &ProxyServerDialog::onNoProxyButtonClicked);
    connect(_systemProxyButton, &CustomRadioButton::clicked, this, &ProxyServerDialog::onSystemProxyButtonClicked);
    connect(_manualProxyButton, &CustomRadioButton::clicked, this, &ProxyServerDialog::onManualProxyButtonClicked);
    connect(_proxyTypeComboBox, QOverload<int>::of(&QComboBox::activated), this, &ProxyServerDialog::onProxyTypeComboBoxActivated);
    connect(_portLineEdit, &QLineEdit::textEdited, this, &ProxyServerDialog::onPortTextEdited);
    connect(_addressLineEdit, &QLineEdit::textEdited, this, &ProxyServerDialog::onAddressTextEdited);
    connect(_authenticationCheckBox, &CustomCheckBox::clicked, this, &ProxyServerDialog::onAuthenticationCheckBoxClicked);
    connect(_loginLineEdit, &QLineEdit::textEdited, this, &ProxyServerDialog::onLoginTextEdited);
    connect(_pwdLineEdit, &QLineEdit::textEdited, this, &ProxyServerDialog::onPwdTextEdited);
}

void ProxyServerDialog::updateUI()
{
    if (_proxyType == QNetworkProxy::NoProxy) {
        if (!_noProxyButton->isChecked()) {
            _noProxyButton->setChecked(true);
        }
    }
    else if (_proxyType == QNetworkProxy::DefaultProxy) {
        if (!_systemProxyButton->isChecked()) {
            _systemProxyButton->setChecked(true);
        }
    }
    else if (!_manualProxyButton->isChecked()) {
        _manualProxyButton->setChecked(true);
    }

    bool manualProxy = _manualProxyButton->isChecked();
    _manualProxyWidget->setVisible(manualProxy);

    if (manualProxy) {
        if (_proxyTypeComboBox->currentIndex() != _manualProxyMap[_proxyType].first) {
            _proxyTypeComboBox->setCurrentIndex(_manualProxyMap[_proxyType].first);
        }
    }
    else {
        _proxyTypeComboBox->setCurrentIndex(-1);
    }

    _portLineEdit->setText(manualProxy
                           ? QString::number(_proxy.port() ? _proxy.port() : defaultPortNumber)
                           : QString());
    _addressLineEdit->setText(manualProxy ? _proxy.hostName() : QString());

    bool authentication = manualProxy && _proxyNeedsAuth;
    if (_authenticationCheckBox->isChecked() != authentication) {
        _authenticationCheckBox->setChecked(authentication);
    }

    _authenticationWidget->setVisible(authentication);

    _loginLineEdit->setText(authentication ? _proxy.user() : QString());
    _pwdLineEdit->setText(authentication ? _proxy.password() : QString());
}

void ProxyServerDialog::setNeedToSave(bool value)
{
    _needToSave = value;
    _saveButton->setEnabled(isSaveEnabled());
}

bool ProxyServerDialog::isSaveEnabled()
{
    bool saveButtonEnabled = _needToSave
            && (_proxyType == QNetworkProxy::NoProxy
                || _proxyType == QNetworkProxy::DefaultProxy
                || (!_proxy.hostName().isEmpty()
                    && (!_proxyNeedsAuth
                        || (!_proxy.user().isEmpty()
                            && !_proxy.password().isEmpty()))));

    return saveButtonEnabled;
}

void ProxyServerDialog::resetManualProxy()
{
    _proxy.setPort(0);
    _proxy.setHostName(QString());
    _proxyNeedsAuth = false;
    resetAuthentication();
}

void ProxyServerDialog::resetAuthentication()
{
    _proxy.setUser(QString());
    _proxy.setPassword(QString());
}

void ProxyServerDialog::onExit()
{
    if (_needToSave) {
        CustomMessageBox *msgBox = new CustomMessageBox(
                    QMessageBox::Question,
                    tr("Do you want to save your modifications?"),
                    QMessageBox::Yes | QMessageBox::No, this);
        msgBox->setDefaultButton(QMessageBox::Yes);
        int ret = msgBox->exec();
        if (ret != QDialog::Rejected) {
            if (ret == QMessageBox::Yes) {
                if (isSaveEnabled()) {
                    onSaveButtonTriggered();
                }
                else {
                    msgBox = new CustomMessageBox(
                                QMessageBox::Warning,
                                tr("Unable to save, all mandatory fields are not completed!"),
                                QMessageBox::Ok, this);
                    msgBox->exec();
                }
            }
            else {
                reject();
            }
        }
    }
    else {
        reject();
    }
}

void ProxyServerDialog::onSaveButtonTriggered(bool checked)
{
    Q_UNUSED(checked)

    // Check host name
    if (!_proxy.hostName().isEmpty()) {
        QHostInfo info = QHostInfo::fromName(_proxy.hostName());
        if (info.error() != QHostInfo::NoError) {
            CustomMessageBox *msgBox = new CustomMessageBox(
                        QMessageBox::Warning,
                        tr("Proxy not found, save anyway?"),
                        QMessageBox::Ok | QMessageBox::Cancel, this);
            msgBox->setDefaultButton(QMessageBox::Cancel);
            int ret = msgBox->exec();
            if (ret != QDialog::Rejected) {
                if (ret == QMessageBox::Cancel) {
                    return;
                }
            }
        }
    }

    OCC::ConfigFile cfgFile;
    cfgFile.setProxyType(_proxyType, _proxy.hostName(), _proxy.port(), _proxyNeedsAuth, _proxy.user(), _proxy.password());

    // Refresh the Qt proxy settings as the quota check can happen all the time
    OCC::ClientProxy proxy;
    proxy.setupQtProxyFromConfig();

    // ... and set the folders dirty, they refresh their proxy next time they start the sync
    OCC::FolderMan::instance()->setDirtyProxy();

    for (auto const &account : OCC::AccountManager::instance()->accounts()) {
        account->freshConnectionAttempt();
    }

    accept();
}

void ProxyServerDialog::onNoProxyButtonClicked(bool checked)
{
    if (checked) {
        _proxyType = QNetworkProxy::NoProxy;
        resetManualProxy();
        updateUI();
        setNeedToSave(true);
    }
}

void ProxyServerDialog::onSystemProxyButtonClicked(bool checked)
{
    if (checked) {
        _proxyType = QNetworkProxy::DefaultProxy;
        resetManualProxy();
        updateUI();
        setNeedToSave(true);
    }
}

void ProxyServerDialog::onManualProxyButtonClicked(bool checked)
{
    if (checked) {
        _proxyType = QNetworkProxy::HttpProxy;
        updateUI();
        setNeedToSave(true);
    }
}

void ProxyServerDialog::onProxyTypeComboBoxActivated(int index)
{
    _proxyType = qvariant_cast<QNetworkProxy::ProxyType>(_proxyTypeComboBox->itemData(index));
    updateUI();
    setNeedToSave(true);
}

void ProxyServerDialog::onPortTextEdited(const QString &text)
{
    _proxy.setPort(text.toInt());
    setNeedToSave(true);
}

void ProxyServerDialog::onAddressTextEdited(const QString &text)
{
    _proxy.setHostName(text);
    setNeedToSave(true);
}

void ProxyServerDialog::onAuthenticationCheckBoxClicked(bool checked)
{
    _proxyNeedsAuth = checked;
    updateUI();
    setNeedToSave(true);

    if (!checked) {
        resetAuthentication();
    }
}

void ProxyServerDialog::onLoginTextEdited(const QString &text)
{
    _proxy.setUser(text);
    setNeedToSave(true);
}

void ProxyServerDialog::onPwdTextEdited(const QString &text)
{
    _proxy.setPassword(text);
    setNeedToSave(true);
}

}
