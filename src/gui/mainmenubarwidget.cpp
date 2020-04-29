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

#include "mainmenubarwidget.h"
#include "guiutility.h"

#include <QLabel>
#include <QPushButton>

namespace KDC {

static const int hMargin = 15;
static const int vMargin = 15;
static const int hSpacing = 30;
static const int logoIconWidth = 100;
static const int logoIconHeight = 38;

MainMenuBarWidget::MainMenuBarWidget(QWidget *parent)
    : HalfRoundRectWidget(parent)
{
    setContentsMargins(hMargin, vMargin, hMargin, vMargin);
    setSpacing(hSpacing);

    QLabel *logoIconLabel = new QLabel(this);
    logoIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/logos/kdrive.svg")
                             .pixmap(QSize(logoIconWidth, logoIconHeight)));
    addWidget(logoIconLabel);
    addStretch();

    QPushButton *drivesRadioButton = new QPushButton(tr("Drives"), this);
    drivesRadioButton->setFlat(true);
    drivesRadioButton->setCheckable(true);
    drivesRadioButton->setAutoExclusive(true);
    drivesRadioButton->setChecked(true);
    addWidget(drivesRadioButton);

    QPushButton *preferencesRadioButton = new QPushButton(tr("Preferences"), this);
    preferencesRadioButton->setFlat(true);
    preferencesRadioButton->setCheckable(true);
    preferencesRadioButton->setAutoExclusive(true);
    addWidget(preferencesRadioButton);

    CustomToolButton *helpButton = new CustomToolButton(this);
    helpButton->setObjectName("helpButton");
    helpButton->setIconPath(":/client/resources/icons/actions/help.svg");
    helpButton->setToolTip(tr("Help"));
    addWidget(helpButton);

    connect(drivesRadioButton, &QPushButton::clicked, this, &MainMenuBarWidget::onDrivesButtonClicked);
    connect(preferencesRadioButton, &QPushButton::clicked, this, &MainMenuBarWidget::onPreferencesButtonClicked);
    connect(helpButton, &CustomToolButton::clicked, this, &MainMenuBarWidget::onHelpButtonClicked);
}

void MainMenuBarWidget::onDrivesButtonClicked(bool checked)
{
    Q_UNUSED(checked)

    emit drivesButtonClicked();
}

void MainMenuBarWidget::onPreferencesButtonClicked(bool checked)
{
    Q_UNUSED(checked)

    emit preferencesButtonClicked();
}

void MainMenuBarWidget::onHelpButtonClicked(bool checked)
{
    Q_UNUSED(checked)
}

}
