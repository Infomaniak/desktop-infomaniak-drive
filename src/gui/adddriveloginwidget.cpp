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

#include "adddriveloginwidget.h"
#include "creds/webflowcredentials.h"
#include "guiutility.h"

#include <QBoxLayout>
#include <QDir>
#include <QSslCertificate>
#include <QSslKey>

namespace KDC {

Q_LOGGING_CATEGORY(lcAddDriveLoginWizard, "adddriveloginwizard", QtInfoMsg)

AddDriveLoginWidget::AddDriveLoginWidget(QWidget *parent)
    : QWidget(parent)
    , _accountPtr(nullptr)
    , _webView(nullptr)
    , _loginUrl(QString())
    , _user(QString())
    , _pass(QString())
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);

    _webView = new OCC::WebView(this);
    mainLayout->addWidget(_webView);

    connect(_webView, &OCC::WebView::urlCatched, this, &AddDriveLoginWidget::urlCatched);
}

void AddDriveLoginWidget::login(const QString &serverUrl)
{
    _loginUrl = serverUrl;
    if (!_loginUrl.endsWith(dirSeparator)) {
        _loginUrl += dirSeparator;
    }
    _loginUrl += "index.php/login/flow";

    qCDebug(lcAddDriveLoginWizard) << "Authentication Url: " << _loginUrl;
    _webView->setUrl(QUrl(_loginUrl));
}

OCC::AbstractCredentials *AddDriveLoginWidget::credentials() const
{
    return new OCC::WebFlowCredentials(_user, _pass, QSslCertificate(), QSslKey());
}

void AddDriveLoginWidget::urlCatched(QString user, QString pass, QString host)
{
    qCDebug(lcAddDriveLoginWizard) << "Got user: " << user << ", server: " << host;

    _user = user;
    _pass = pass;

    if (!_accountPtr.isNull()) {
        _accountPtr->setUrl(host);
        emit terminated();
    }
}

}

