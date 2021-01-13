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

#include "adddriveconfirmationwidget.h"
#include "custommessagebox.h"
#include "customtoolbutton.h"
#include "guiutility.h"
#include "wizard/owncloudwizardcommon.h"

#include <QBoxLayout>
#include <QDir>
#include <QFileDialog>
#include <QLoggingCategory>
#include <QProgressBar>

namespace KDC {

static const int boxHSpacing = 10;
static const int logoBoxVMargin = 20;
static const int progressBarVMargin = 90;
static const QSize _okIconSize = QSize(60, 60);
static const int okIconVMargin = 30;
static const int hLogoSpacing = 20;
static const int logoIconSize = 39;
static const QSize logoTextIconSize = QSize(60, 42);
static const int titleBoxVMargin = 30;
static const int descriptionVMargin = 50;
static const int buttonBoxVMargin = 20;
static const int progressBarMin = 0;
static const int progressBarMax = 4;

Q_LOGGING_CATEGORY(lcAddDriveConfirmationWidget, "adddrivelocalfolderwidget", QtInfoMsg)

AddDriveConfirmationWidget::AddDriveConfirmationWidget(QWidget *parent)
    : QWidget(parent)
    , _logoTextIconLabel(nullptr)
    , _descriptionLabel(nullptr)
    , _logoColor(QColor())
{
    initUI();
}

void AddDriveConfirmationWidget::setFolderPath(const QString &path)
{
    QDir dir(path);
    _descriptionLabel->setText(tr("Synchronization will start and you will be able to add files to your %1 folder.")
                               .arg(dir.dirName()));
}

void AddDriveConfirmationWidget::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    setLayout(mainLayout);

    // Logo
    QHBoxLayout *logoHBox = new QHBoxLayout();
    logoHBox->setContentsMargins(0, 0, 0, 0);
    mainLayout->addLayout(logoHBox);
    mainLayout->addSpacing(logoBoxVMargin);

    QLabel *logoIconLabel = new QLabel(this);
    logoIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/logos/kdrive-without-text.svg")
                             .pixmap(QSize(logoIconSize, logoIconSize)));
    logoHBox->addWidget(logoIconLabel);
    logoHBox->addSpacing(hLogoSpacing);

    _logoTextIconLabel = new QLabel(this);
    logoHBox->addWidget(_logoTextIconLabel);
    logoHBox->addStretch();

    // Progress bar
    QProgressBar *progressBar = new QProgressBar(this);
    progressBar->setMinimum(progressBarMin);
    progressBar->setMaximum(progressBarMax);
    progressBar->setValue(4);
    progressBar->setFormat(QString());
    mainLayout->addWidget(progressBar);
    mainLayout->addSpacing(progressBarVMargin);

    // OK icon
    QLabel *okIconLabel = new QLabel(this);
    okIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/statuts/success.svg").pixmap(_okIconSize));
    okIconLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(okIconLabel);
    mainLayout->addSpacing(okIconVMargin);

    // Title
    QLabel *titleLabel = new QLabel(this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setText(tr("Your kDrive is ready!"));
    titleLabel->setContentsMargins(0, 0, 0, 0);
    titleLabel->setWordWrap(true);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(titleBoxVMargin);

    // Description
    _descriptionLabel = new QLabel(this);
    _descriptionLabel->setObjectName("largeNormalTextLabel");
    _descriptionLabel->setWordWrap(true);
    _descriptionLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(_descriptionLabel);
    mainLayout->addSpacing(descriptionVMargin);

    // Add dialog buttons
    QHBoxLayout *buttonsHBox = new QHBoxLayout();
    buttonsHBox->setContentsMargins(0, 0, 0, 0);
    buttonsHBox->setSpacing(boxHSpacing);
    mainLayout->addLayout(buttonsHBox);

    QPushButton *openFolderButton = new QPushButton(this);
    openFolderButton->setObjectName("defaultbutton");
    openFolderButton->setFlat(true);
    openFolderButton->setText(tr("OPEN FOLDER"));
    buttonsHBox->addStretch();
    buttonsHBox->addWidget(openFolderButton);

    QPushButton *openParametersButton = new QPushButton(this);
    openParametersButton->setObjectName("nondefaultbutton");
    openParametersButton->setFlat(true);
    openParametersButton->setText(tr("PARAMETERS"));
    buttonsHBox->addWidget(openParametersButton);
    buttonsHBox->addStretch();
    mainLayout->addSpacing(buttonBoxVMargin);

    QPushButton *addDriveButton = new QPushButton(this);
    addDriveButton->setObjectName("transparentbutton");
    addDriveButton->setFlat(true);
    addDriveButton->setText(tr("Synchronize another drive"));
    mainLayout->addWidget(addDriveButton);
    mainLayout->addStretch();

    connect(openFolderButton, &QPushButton::clicked, this, &AddDriveConfirmationWidget::onOpenFoldersButtonTriggered);
    connect(openParametersButton, &QPushButton::clicked, this, &AddDriveConfirmationWidget::onOpenParametersButtonTriggered);
    connect(addDriveButton, &QPushButton::clicked, this, &AddDriveConfirmationWidget::onAddDriveButtonTriggered);
}

void AddDriveConfirmationWidget::setLogoColor(const QColor &color)
{
    _logoColor = color;
    _logoTextIconLabel->setPixmap(
                OCC::Utility::getIconWithColor(":/client/resources/logos/kdrive-text-only.svg", _logoColor)
                .pixmap(logoTextIconSize));
}

void AddDriveConfirmationWidget::onOpenFoldersButtonTriggered(bool checked)
{
    Q_UNUSED(checked)

    _action = OCC::Utility::WizardAction::OpenFolder;
    emit terminated();
}

void AddDriveConfirmationWidget::onOpenParametersButtonTriggered(bool checked)
{
    Q_UNUSED(checked)

    _action = OCC::Utility::WizardAction::OpenParameters;
    emit terminated();
}

void AddDriveConfirmationWidget::onAddDriveButtonTriggered(bool checked)
{
    Q_UNUSED(checked)

    _action = OCC::Utility::WizardAction::AddDrive;
    emit terminated();
}

}

