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

#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>

namespace KDC {

static const int hMargin = 10;
static const int vMargin = 0;
static const int driveBoxHMargin = 10;
static const int driveBoxVMargin = 10;
static const int hButtonsSpacing = 50;

MainMenuBarWidget::MainMenuBarWidget(QWidget *parent)
    : HalfRoundRectWidget(parent)
    , _driveSelectionWidget(nullptr)
    , _progressBarWidget(nullptr)
{
    setContentsMargins(hMargin, vMargin, hMargin, vMargin);
    setSpacing(0);

    QVBoxLayout *vBox = new QVBoxLayout();
    vBox->setContentsMargins(0, 0, 0, 0);
    addLayout(vBox);

    // 1st line
    QHBoxLayout *hBox = new QHBoxLayout();
    hBox->setContentsMargins(driveBoxHMargin, driveBoxVMargin, driveBoxHMargin, driveBoxVMargin);
    vBox->addLayout(hBox);

    // Drive selection
    _driveSelectionWidget = new DriveSelectionWidget(this);
    hBox->addWidget(_driveSelectionWidget);
    hBox->addStretch();

    // Preferences button
    QPushButton *preferencesButton = new QPushButton(tr("Preferences"), this);
    preferencesButton->setObjectName("preferencesButton");
    preferencesButton->setFlat(true);
    hBox->addWidget(preferencesButton);
    hBox->addSpacing(hButtonsSpacing);

    // Help button
    CustomToolButton *helpButton = new CustomToolButton(this);
    helpButton->setObjectName("helpButton");
    helpButton->setIconPath(":/client/resources/icons/actions/help.svg");
    helpButton->setToolTip(tr("Help"));
    hBox->addWidget(helpButton);

    // Quota line
    QHBoxLayout *quotaHBox = new QHBoxLayout();
    quotaHBox->setContentsMargins(0, 0, 0, 0);
    vBox->addLayout(quotaHBox);
    vBox->addStretch();

    // Progress bar
    _progressBarWidget = new ProgressBarWidget(this);
    quotaHBox->addWidget(_progressBarWidget);

    connect(_driveSelectionWidget, &DriveSelectionWidget::driveSelected, this, &MainMenuBarWidget::onAccountSelected);
    connect(_driveSelectionWidget, &DriveSelectionWidget::addDrive, this, &MainMenuBarWidget::onAddDrive);
    connect(preferencesButton, &QPushButton::clicked, this, &MainMenuBarWidget::onPreferencesButtonClicked);
    connect(helpButton, &CustomToolButton::clicked, this, &MainMenuBarWidget::onHelpButtonClicked);
}

void MainMenuBarWidget::onAccountSelected(QString id)
{
    emit accountSelected(id);
}

void MainMenuBarWidget::onAddDrive()
{
    emit addDrive();
}

void MainMenuBarWidget::onPreferencesButtonClicked(bool checked)
{
    Q_UNUSED(checked)

    emit preferencesButtonClicked();
}

void MainMenuBarWidget::onHelpButtonClicked(bool checked)
{
    Q_UNUSED(checked)

    emit openHelp();
}

}
