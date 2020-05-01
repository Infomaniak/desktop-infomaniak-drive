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

#include "driveswidget.h"
#include "adddrivewidget.h"
#include "accountitemwidget.h"

#include <QBoxLayout>

namespace KDC {

static const int addDriveBoxHMargin= 12;
static const int addDriveBoxVMargin = 10;

DrivesWidget::DrivesWidget(QWidget *parent)
    : QWidget(parent)
    , _driveListWidget(nullptr)
{
    setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *mainVBox = new QVBoxLayout();
    mainVBox->setContentsMargins(0, 0, 0, 0);
    mainVBox->setSpacing(0);
    setLayout(mainVBox);

    // Add drive widget
    QHBoxLayout *addDriveHBox = new QHBoxLayout();
    addDriveHBox->setContentsMargins(addDriveBoxHMargin, addDriveBoxVMargin, addDriveBoxHMargin, addDriveBoxVMargin);

    AddDriveWidget *addDriveWidget = new AddDriveWidget(this);
    addDriveHBox->addWidget(addDriveWidget);
    addDriveHBox->addStretch();
    mainVBox->addLayout(addDriveHBox);

    // Drive list
    _driveListWidget = new QListWidget(this);
    _driveListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    _driveListWidget->setSpacing(0);
    mainVBox->addWidget(_driveListWidget);
    mainVBox->setStretchFactor(_driveListWidget, 1);

    connect(addDriveWidget, &AddDriveWidget::addDrive, this, &DrivesWidget::onAddDrive);
}

void DrivesWidget::clear()
{
    _driveMap.clear();
    _driveListWidget->clear();
}

void DrivesWidget::addOrUpdateDrive(QString accountId, const AccountInfo &accountInfo)
{
    auto driveIt = _driveMap.find(accountId);
    if (driveIt == _driveMap.end()) {
        // Add drive
        AccountInfoDrivesWidget accountInfoDrivesWidget(accountInfo);
        accountInfoDrivesWidget._item = new QListWidgetItem();
        AccountItemWidget *accountItemWidget = new AccountItemWidget(accountId, _driveListWidget);
        accountItemWidget->updateItem(accountInfo);
        _driveListWidget->insertItem(0, accountInfoDrivesWidget._item);
        _driveListWidget->setItemWidget(accountInfoDrivesWidget._item, accountItemWidget);
        _driveMap[accountId] = accountInfoDrivesWidget;

        connect(accountItemWidget, &AccountItemWidget::runSync, this, &DrivesWidget::onRunSync);
        connect(accountItemWidget, &AccountItemWidget::pauseSync, this, &DrivesWidget::onPauseSync);
        connect(accountItemWidget, &AccountItemWidget::resumeSync, this, &DrivesWidget::onResumeSync);
        connect(accountItemWidget, &AccountItemWidget::remove, this, &DrivesWidget::onRemove);
    }
    else {
        // Update drive
        AccountItemWidget *accountItemWidget = (AccountItemWidget *) _driveListWidget->itemWidget(driveIt->second._item);
        accountItemWidget->updateItem(accountInfo);
    }
}

void DrivesWidget::removeDrive(QString accountId)
{
    auto driveIt = _driveMap.find(accountId);
    if (driveIt != _driveMap.end()) {
        // Remove drive
        if (driveIt->second._item) {
            int row = _driveListWidget->row(driveIt->second._item);
            _driveListWidget->takeItem(row);
            delete driveIt->second._item;

            _driveMap.erase(driveIt);
        }
    }
}

void DrivesWidget::onAddDrive()
{
    emit addDrive();
}

void DrivesWidget::onRunSync(const QString &accountId)
{
    emit runSync(accountId);
}

void DrivesWidget::onPauseSync(const QString &accountId)
{
    emit pauseSync(accountId);
}

void DrivesWidget::onResumeSync(const QString &accountId)
{
    emit resumeSync(accountId);
}

void DrivesWidget::onRemove(const QString &accountId)
{
    emit remove(accountId);
}

DrivesWidget::AccountInfoDrivesWidget::AccountInfoDrivesWidget()
    : AccountInfo()
    , _item(nullptr)
{
}

DrivesWidget::AccountInfoDrivesWidget::AccountInfoDrivesWidget(const AccountInfo &accountInfo)
    : AccountInfo(accountInfo)
    , _item(nullptr)
{
}

}
