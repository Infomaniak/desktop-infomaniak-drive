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

#include "wizard/webview.h"
#include "account.h"
#include "creds/abstractcredentials.h"

#include <QString>
#include <QWidget>

namespace KDC {

class AddDriveLoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AddDriveLoginWidget(QWidget *parent = nullptr);

    inline void setAccountPtr(OCC::AccountPtr accountPtr) { _accountPtr = accountPtr; }
    void login(const QString &serverUrl);
    inline QString loginUrl() const { return _loginUrl; }
    OCC::AbstractCredentials *credentials() const;

signals:
    void terminated(bool next = true);

private:
    OCC::AccountPtr _accountPtr;
    OCC::WebView *_webView;
    QString _loginUrl;
    QString _user;
    QString _pass;

private slots:
    void urlCatched(QString user, QString pass, QString host);
};

}

