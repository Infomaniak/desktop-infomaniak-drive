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

#include "errorsmenubarwidget.h"
#include "customtoolbutton.h"
#include "guiutility.h"

namespace KDC {

static const int hMargin = 15;
static const int vMargin = 15;
static const int hButtonsSpacing = 10;
static const int driveLogoIconSize = 24;

ErrorsMenuBarWidget::ErrorsMenuBarWidget(QWidget *parent)
    : HalfRoundRectWidget(parent)
    , _accountId(QString())
    , _accountInfo(nullptr)
    , _accountIconLabel(nullptr)
    , _titleLabel(nullptr)
{
    setContentsMargins(hMargin, 0, hMargin, vMargin);
    setSpacing(0);

    CustomToolButton *backButton = new CustomToolButton(this);
    backButton->setIconPath(":/client/resources/icons/actions/arrow-left.svg");
    backButton->setToolTip(tr("Back to drive preferences"));
    addWidget(backButton);

    addSpacing(hButtonsSpacing);

    _accountIconLabel = new QLabel(this);
    addWidget(_accountIconLabel);

    addSpacing(hButtonsSpacing);

    _titleLabel = new QLabel(this);
    _titleLabel->setObjectName("titleLabel");
    _titleLabel->setText(tr("Synchronization errors"));
    addWidget(_titleLabel);

    addStretch();

    connect(backButton, &CustomToolButton::clicked, this, &ErrorsMenuBarWidget::onBackButtonClicked);
}

void ErrorsMenuBarWidget::setAccount(const QString &accountId, const AccountInfo *accountInfo)
{
    _accountId = accountId;
    _accountInfo = accountInfo;
    _accountIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/drive.svg",
                                                                _accountInfo->_color).
                                 pixmap(QSize(driveLogoIconSize, driveLogoIconSize)));
}

void ErrorsMenuBarWidget::reset()
{
    _accountId = QString();
    _accountInfo = nullptr;
}

void ErrorsMenuBarWidget::onBackButtonClicked(bool checked)
{
    Q_UNUSED(checked)

    emit backButtonClicked();
}

}
