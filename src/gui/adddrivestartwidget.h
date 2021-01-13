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

#include "account.h"
#include "wizard/postfixlineedit.h"

#include <QPushButton>
#include <QWidget>

namespace KDC {

class AddDriveStartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AddDriveStartWidget(bool autoNext, QWidget *parent = nullptr);

    inline void setAccountPtr(OCC::AccountPtr accountPtr) { _accountPtr = accountPtr; }
    void setServerUrl(const QString &url);
    QString serverUrl() const;

signals:
    void terminated(bool next = true);

private:
    bool _autoNext;
    OCC::AccountPtr _accountPtr;
    OCC::PostfixLineEdit *_serverUrlLineEdit;
    QPushButton *_nextButton;

private slots:
    void onUrlChanged(const QString &text);
    void onUrlEditFinished();
    void onNextButtonTriggered(bool checked = false);
};

}

