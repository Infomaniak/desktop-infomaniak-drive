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

#include "adddrivelocalfolderwidget.h"
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
static const int progressBarVMargin = 40;
static const int hLogoSpacing = 20;
static const int logoIconSize = 39;
static const QSize logoTextIconSize = QSize(60, 42);
static const int titleBoxVMargin = 30;
static const int selectionBoxHMargin = 15;
static const int selectionBoxVMargin = 20;
static const int selectionBoxSpacing = 10;
static const int selectionWidgetVMargin = 30;
static const int progressBarMin = 0;
static const int progressBarMax = 4;

Q_LOGGING_CATEGORY(lcAddDriveLocalFolderWidget, "adddrivelocalfolderwidget", QtInfoMsg)

AddDriveLocalFolderWidget::AddDriveLocalFolderWidget(QWidget *parent)
    : QWidget(parent)
    , _localFolderPath(QString())
    , _logoTextIconLabel(nullptr)
    , _titleLabel(nullptr)
    , _folderIconLabel(nullptr)
    , _folderNameLabel(nullptr)
    , _folderPathLabel(nullptr)
    , _endButton(nullptr)
    , _folderIconColor(QColor())
    , _folderIconSize(QSize())
    , _logoColor(QColor())
    , _needToSave(false)
{
    initUI();
    updateUI();
}

void AddDriveLocalFolderWidget::setAccountPtr(OCC::AccountPtr accountPtr)
{
    _titleLabel->setText(tr("Location of your %1 kDrive").arg(accountPtr->driveName()));
}

void AddDriveLocalFolderWidget::setLocalFolderPath(const QString &path)
{
    _localFolderPath = path;
    bool ok = !_localFolderPath.isEmpty();
    if (ok) {
        QDir dir(_localFolderPath);
        _folderNameLabel->setText(dir.dirName());
        _folderPathLabel->setText(QString("<a style=\"%1\" href=\"ref\">%2</a>")
                                  .arg(OCC::Utility::linkStyle)
                                  .arg(_localFolderPath));
    }
}

void AddDriveLocalFolderWidget::initUI()
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
    progressBar->setValue(3);
    progressBar->setFormat(QString());
    mainLayout->addWidget(progressBar);
    mainLayout->addSpacing(progressBarVMargin);

    // Title
    _titleLabel = new QLabel(this);
    _titleLabel->setObjectName("titleLabel");
    _titleLabel->setContentsMargins(0, 0, 0, 0);
    _titleLabel->setWordWrap(true);
    mainLayout->addWidget(_titleLabel);
    mainLayout->addSpacing(titleBoxVMargin);

    // Folder selected widget
    QWidget *folderSelectedWidget = new QWidget(this);
    folderSelectedWidget->setObjectName("folderSelectionWidget");
    mainLayout->addWidget(folderSelectedWidget);

    QHBoxLayout *folderSelectedHBox = new QHBoxLayout();
    folderSelectedHBox->setContentsMargins(selectionBoxHMargin, selectionBoxVMargin, selectionBoxHMargin, selectionBoxVMargin);
    folderSelectedWidget->setLayout(folderSelectedHBox);

    QVBoxLayout *folderSelectedVBox = new QVBoxLayout();
    folderSelectedVBox->setContentsMargins(0, 0, 0, 0);
    folderSelectedHBox->addLayout(folderSelectedVBox);
    folderSelectedHBox->addStretch();

    QHBoxLayout *folderNameSelectedHBox = new QHBoxLayout();
    folderNameSelectedHBox->setContentsMargins(0, 0, 0, 0);
    folderNameSelectedHBox->setSpacing(selectionBoxSpacing);
    folderSelectedVBox->addLayout(folderNameSelectedHBox);

    _folderIconLabel = new QLabel(this);
    folderNameSelectedHBox->addWidget(_folderIconLabel);

    _folderNameLabel = new QLabel(this);
    _folderNameLabel->setObjectName("foldernamelabel");
    folderNameSelectedHBox->addWidget(_folderNameLabel);
    folderNameSelectedHBox->addStretch();

    _folderPathLabel = new QLabel(this);
    _folderPathLabel->setObjectName("folderpathlabel");
    folderSelectedVBox->addWidget(_folderPathLabel);

    CustomToolButton *updateButton = new CustomToolButton(this);
    updateButton->setIconPath(":/client/resources/icons/actions/edit.svg");
    updateButton->setToolTip(tr("Edit folder"));
    folderSelectedHBox->addWidget(updateButton);
    mainLayout->addSpacing(selectionWidgetVMargin);

    // Description
    QLabel *descriptionLabel = new QLabel(this);
    descriptionLabel->setObjectName("largeNormalTextLabel");
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setText(tr("You will find all your files in this folder when the configuration is complete.<br>"
                                 "You can drop new files there to sync them to your kDrive."));
    mainLayout->addWidget(descriptionLabel);
    mainLayout->addStretch();

    // Add dialog buttons
    QHBoxLayout *buttonsHBox = new QHBoxLayout();
    buttonsHBox->setContentsMargins(0, 0, 0, 0);
    buttonsHBox->setSpacing(boxHSpacing);
    mainLayout->addLayout(buttonsHBox);

    QPushButton *backButton = new QPushButton(this);
    backButton->setObjectName("nondefaultbutton");
    backButton->setFlat(true);
    backButton->setIcon(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/chevron-left.svg"));
    buttonsHBox->addWidget(backButton);
    buttonsHBox->addStretch();

    _endButton = new QPushButton(this);
    _endButton->setObjectName("defaultbutton");
    _endButton->setFlat(true);
    _endButton->setText(tr("END"));
    buttonsHBox->addWidget(_endButton);

    connect(updateButton, &CustomToolButton::clicked, this, &AddDriveLocalFolderWidget::onUpdateFolderButtonTriggered);
    connect(_folderPathLabel, &QLabel::linkActivated, this, &AddDriveLocalFolderWidget::onLinkActivated);
    connect(backButton, &QPushButton::clicked, this, &AddDriveLocalFolderWidget::onBackButtonTriggered);
    connect(_endButton, &QPushButton::clicked, this, &AddDriveLocalFolderWidget::onContinueButtonTriggered);
}

void AddDriveLocalFolderWidget::updateUI()
{
    bool ok = !_localFolderPath.isEmpty();
    if (ok) {
        QDir dir(_localFolderPath);
        _folderNameLabel->setText(dir.dirName());
        _folderPathLabel->setText(QString("<a style=\"%1\" href=\"ref\">%2</a>")
                                  .arg(OCC::Utility::linkStyle)
                                  .arg(_localFolderPath));
    }
}

void AddDriveLocalFolderWidget::selectFolder(const QString &startDirPath)
{
    QString dirPath = QFileDialog::getExistingDirectory(this, tr("Select folder"), startDirPath,
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dirPath.isEmpty()) {
        QDir dir(dirPath);
        _localFolderPath = dir.canonicalPath();
        updateUI();
    }
}

void AddDriveLocalFolderWidget::setFolderIcon()
{
    if (_folderIconColor != QColor() && _folderIconSize != QSize()) {
        _folderIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/folder.svg", _folderIconColor)
                                   .pixmap(_folderIconSize));
    }
}

void AddDriveLocalFolderWidget::setLogoColor(const QColor &color)
{
    _logoColor = color;
    _logoTextIconLabel->setPixmap(
                OCC::Utility::getIconWithColor(":/client/resources/logos/kdrive-text-only.svg", _logoColor)
                .pixmap(logoTextIconSize));
}

void AddDriveLocalFolderWidget::onDisplayMessage(const QString &text)
{
    CustomMessageBox *msgBox = new CustomMessageBox(
                QMessageBox::Warning,
                text,
                QMessageBox::Ok, this);
    msgBox->setDefaultButton(QMessageBox::Ok);
    msgBox->exec();
}

void AddDriveLocalFolderWidget::onNeedToSave()
{
    _needToSave = true;
}

void AddDriveLocalFolderWidget::onBackButtonTriggered(bool checked)
{
    Q_UNUSED(checked)

    emit terminated(false);
}

void AddDriveLocalFolderWidget::onContinueButtonTriggered(bool checked)
{
    Q_UNUSED(checked)

    emit terminated();
}

void AddDriveLocalFolderWidget::onUpdateFolderButtonTriggered(bool checked)
{
    Q_UNUSED(checked)

    selectFolder(_localFolderPath);
}

void AddDriveLocalFolderWidget::onLinkActivated(const QString &link)
{
    Q_UNUSED(link)

    //emit openFolder(_localFolderPath);
}

}

