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
static const int vMargin = 10;
static const int hLogoSpacing = 10;
static const int hButtonsSpacing = 30;
static const int logoIconSize = 39;
static const QSize logoTextIconSize = QSize(60, 42);

MainMenuBarWidget::MainMenuBarWidget(QWidget *parent)
    : HalfRoundRectWidget(parent)
    , _logoColor(QColor())
    , _logoTextIconLabel(nullptr)
{
    setContentsMargins(hMargin, vMargin, hMargin, vMargin);
    setSpacing(0);

    QLabel *logoIconLabel = new QLabel(this);
    logoIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/logos/kdrive-without-text.svg")
                             .pixmap(QSize(logoIconSize, logoIconSize)));
    addWidget(logoIconLabel);
    addSpacing(hLogoSpacing);

    _logoTextIconLabel = new QLabel(this);
    addWidget(_logoTextIconLabel);

    addStretch();

    QPushButton *drivesRadioButton = new QPushButton(tr("Drives"), this);
    drivesRadioButton->setFlat(true);
    drivesRadioButton->setCheckable(true);
    drivesRadioButton->setAutoExclusive(true);
    drivesRadioButton->setChecked(true);
    addWidget(drivesRadioButton);
    addSpacing(hButtonsSpacing);

    QPushButton *preferencesRadioButton = new QPushButton(tr("Preferences"), this);
    preferencesRadioButton->setFlat(true);
    preferencesRadioButton->setCheckable(true);
    preferencesRadioButton->setAutoExclusive(true);
    addWidget(preferencesRadioButton);
    addSpacing(hButtonsSpacing);

    CustomToolButton *helpButton = new CustomToolButton(this);
    helpButton->setObjectName("helpButton");
    helpButton->setIconPath(":/client/resources/icons/actions/help.svg");
    helpButton->setToolTip(tr("Help"));
    addWidget(helpButton);

    connect(this, &MainMenuBarWidget::logoColorChanged, this, &MainMenuBarWidget::onLogoColorChanged);
    connect(drivesRadioButton, &QPushButton::clicked, this, &MainMenuBarWidget::onDrivesButtonClicked);
    connect(preferencesRadioButton, &QPushButton::clicked, this, &MainMenuBarWidget::onPreferencesButtonClicked);
    connect(helpButton, &CustomToolButton::clicked, this, &MainMenuBarWidget::onHelpButtonClicked);
}

void MainMenuBarWidget::onLogoColorChanged()
{
    _logoTextIconLabel->setPixmap(
                OCC::Utility::getIconWithColor(":/client/resources/logos/kdrive-text-only.svg", _logoColor)
                .pixmap(logoTextIconSize));
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
