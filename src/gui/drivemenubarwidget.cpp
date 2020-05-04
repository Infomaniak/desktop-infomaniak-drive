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

#include "drivemenubarwidget.h"
#include "customtoolbutton.h"
#include "guiutility.h"

#include <QSize>

namespace KDC {

static const int hMargin = 15;
static const int vMargin = 10;
static const int hButtonsSpacing = 10;

DriveMenuBarWidget::DriveMenuBarWidget(QWidget *parent)
    : HalfRoundRectWidget(parent)
    , _accountIconLabel(nullptr)
    , _accountNameLabel(nullptr)
{
    setContentsMargins(hMargin, vMargin, hMargin, vMargin);
    setSpacing(0);

    CustomToolButton *backButton = new CustomToolButton(this);
    backButton->setIconPath(":/client/resources/icons/actions/arrow-left.svg");
    backButton->setToolTip(tr("Back to drive list"));
    addWidget(backButton);

    addSpacing(hButtonsSpacing);

    _accountIconLabel = new QLabel(this);
    addWidget(_accountIconLabel);

    addSpacing(hButtonsSpacing);

    _accountNameLabel = new QLabel(this);
    _accountNameLabel->setObjectName("accountNameLabel");
    addWidget(_accountNameLabel);

    addStretch();

    CustomToolButton *menuButton = new CustomToolButton(this);
    menuButton->setObjectName("backButton");
    menuButton->setIconPath(":/client/resources/icons/actions/menu.svg");
    menuButton->setToolTip(tr("More actions"));
    addWidget(menuButton);

    connect(backButton, &CustomToolButton::clicked, this, &DriveMenuBarWidget::onBackButtonClicked);
}

void DriveMenuBarWidget::setAccount(const QColor &color, const QString &name)
{
    _accountIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/drive.svg", color).
                                 pixmap(QSize(24, 24)));
    _accountNameLabel->setText(name);
}

void DriveMenuBarWidget::onBackButtonClicked(bool checked)
{
    Q_UNUSED(checked)

    emit backButtonClicked();
}

}
