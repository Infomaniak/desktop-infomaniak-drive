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
#include "accountinfo.h"

#include <map>

#include <QLabel>
#include <QString>
#include <QWidget>

namespace KDC {

class DriveMenuBarWidget : public HalfRoundRectWidget
{
    Q_OBJECT

public:
    explicit DriveMenuBarWidget(QWidget *parent = nullptr);

    void setAccount(std::map<QString, AccountInfo>::iterator accountInfoIt);
    void reset();

signals:
    void backButtonClicked();
    void runSync(const QString &accountId);
    void pauseSync(const QString &accountId);
    void resumeSync(const QString &accountId);
    void manageOffer(const QString &accountId);
    void remove(const QString &accountId);

private:
    QString _accountId;
    AccountInfo *_accountInfo;
    QLabel *_accountIconLabel;
    QLabel *_accountNameLabel;
    CustomToolButton *_menuButton;

    void onBackButtonClicked(bool checked = false);
    void onMenuButtonClicked(bool checked = false);
    void onSyncTriggered();
    void onPauseTriggered();
    void onResumeTriggered();
    void onManageOfferTriggered();
    void onRemoveTriggered();
};

}

