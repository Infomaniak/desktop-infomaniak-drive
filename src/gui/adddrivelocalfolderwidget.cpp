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
#include "common/vfs.h"
#include "configfile.h"

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
static const int infoBoxSpacing = 10;
static const int infoWidgetVMargin = 25;
static const int progressBarMin = 0;
static const int progressBarMax = 4;

Q_LOGGING_CATEGORY(lcAddDriveLocalFolderWidget, "gui.adddrivelocalfolderwidget", QtInfoMsg)

AddDriveLocalFolderWidget::AddDriveLocalFolderWidget(QWidget *parent)
    : QWidget(parent)
    , _localFolderPath(QString())
    , _defaultLocalFolderPath(QString())
    , _logoTextIconLabel(nullptr)
    , _titleLabel(nullptr)
    , _folderIconLabel(nullptr)
    , _folderNameLabel(nullptr)
    , _folderPathLabel(nullptr)
    , _warningWidget(nullptr)
    , _warningIconLabel(nullptr)
    , _warningLabel(nullptr)
    , _infoWidget(nullptr)
    , _infoIconLabel(nullptr)
    , _infoLabel(nullptr)
    , _backButton(nullptr)
    , _endButton(nullptr)
    , _folderIconColor(QColor())
    , _folderIconSize(QSize())
    , _infoIconColor(QColor())
    , _infoIconSize(QSize())
    , _warningIconColor(QColor())
    , _warningIconSize(QSize())
    , _logoColor(QColor())
    , _needToSave(false)
    , _smartSync(false)
    , _folderCompatibleWithSmartSync(false)
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
    _defaultLocalFolderPath = path;
    updateUI();
}

void AddDriveLocalFolderWidget::setButtonIcon(const QColor &value)
{
    if (_backButton) {
        _backButton->setIcon(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/chevron-left.svg", value));
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
    _folderNameLabel->setObjectName("folderNameLabel");
    folderNameSelectedHBox->addWidget(_folderNameLabel);
    folderNameSelectedHBox->addStretch();

    _folderPathLabel = new QLabel(this);
    _folderPathLabel->setObjectName("folderpathlabel");
    _folderPathLabel->setContextMenuPolicy(Qt::PreventContextMenu);
    folderSelectedVBox->addWidget(_folderPathLabel);

    CustomToolButton *updateButton = new CustomToolButton(this);
    updateButton->setIconPath(":/client/resources/icons/actions/edit.svg");
    updateButton->setToolTip(tr("Edit folder"));
    folderSelectedHBox->addWidget(updateButton);
    mainLayout->addSpacing(selectionWidgetVMargin);

    // Info
    _infoWidget = new QWidget(this);
    _infoWidget->setVisible(false);
    mainLayout->addWidget(_infoWidget);

    QVBoxLayout *infoVBox = new QVBoxLayout();
    infoVBox->setContentsMargins(0, 0, 0, 0);
    _infoWidget->setLayout(infoVBox);

    QHBoxLayout *infoHBox = new QHBoxLayout();
    infoHBox->setContentsMargins(0, 0, 0, 0);
    infoHBox->setSpacing(infoBoxSpacing);
    infoVBox->addLayout(infoHBox);
    infoVBox->addSpacing(infoWidgetVMargin);

    _infoIconLabel = new QLabel(this);
    infoHBox->addWidget(_infoIconLabel);

    _infoLabel = new QLabel(this);
    _infoLabel->setObjectName("largeMediumTextLabel");
    _infoLabel->setWordWrap(true);
    infoHBox->addWidget(_infoLabel);
    infoHBox->setStretchFactor(_infoLabel, 1);

    // Warning
    _warningWidget = new QWidget(this);
    _warningWidget->setVisible(false);
    mainLayout->addWidget(_warningWidget);

    QVBoxLayout *warningVBox = new QVBoxLayout();
    warningVBox->setContentsMargins(0, 0, 0, 0);
    _warningWidget->setLayout(warningVBox);

    QHBoxLayout *warningHBox = new QHBoxLayout();
    warningHBox->setContentsMargins(0, 0, 0, 0);
    warningHBox->setSpacing(infoBoxSpacing);
    warningVBox->addLayout(warningHBox);
    warningVBox->addSpacing(infoWidgetVMargin);

    _warningIconLabel = new QLabel(this);
    warningHBox->addWidget(_warningIconLabel);

    _warningLabel = new QLabel(this);
    _warningLabel->setObjectName("largeMediumTextLabel");
    _warningLabel->setWordWrap(true);
    warningHBox->addWidget(_warningLabel);
    warningHBox->setStretchFactor(_warningLabel, 1);

    // Description
    QLabel *descriptionLabel = new QLabel(this);
    descriptionLabel->setObjectName("largeNormalTextLabel");
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setText(tr("You will find all your files in this folder when the configuration is complete."
                                 " You can drop new files there to sync them to your kDrive."));
    mainLayout->addWidget(descriptionLabel);
    mainLayout->addStretch();

    // Add dialog buttons
    QHBoxLayout *buttonsHBox = new QHBoxLayout();
    buttonsHBox->setContentsMargins(0, 0, 0, 0);
    buttonsHBox->setSpacing(boxHSpacing);
    mainLayout->addLayout(buttonsHBox);

    _backButton = new QPushButton(this);
    _backButton->setObjectName("nondefaultbutton");
    _backButton->setFlat(true);
    buttonsHBox->addWidget(_backButton);
    buttonsHBox->addStretch();

    _endButton = new QPushButton(this);
    _endButton->setObjectName("defaultbutton");
    _endButton->setFlat(true);
    _endButton->setText(tr("END"));
    buttonsHBox->addWidget(_endButton);

    connect(updateButton, &CustomToolButton::clicked, this, &AddDriveLocalFolderWidget::onUpdateFolderButtonTriggered);
    connect(_folderPathLabel, &QLabel::linkActivated, this, &AddDriveLocalFolderWidget::onLinkActivated);
    connect(_warningLabel, &QLabel::linkActivated, this, &AddDriveLocalFolderWidget::onLinkActivated);
    connect(_backButton, &QPushButton::clicked, this, &AddDriveLocalFolderWidget::onBackButtonTriggered);
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

        if (_localFolderPath != _defaultLocalFolderPath) {
            _infoLabel->setText(tr("The contents of the <b>%1</b> folder will be synchronized in your kDrive")
                                .arg(dir.dirName()));
            _infoWidget->setVisible(true);
        }
        else {
            _infoWidget->setVisible(false);
        }

        if (_smartSync) {
            OCC::Vfs::Mode mode = OCC::bestAvailableVfsMode(OCC::ConfigFile().showExperimentalOptions());
            if (mode == OCC::Vfs::WindowsCfApi) {
                // Check file system
                QString fsName(OCC::Utility::fileSystemName(dir.rootPath()));
                _folderCompatibleWithSmartSync = (fsName == "NTFS");
                if (!_folderCompatibleWithSmartSync) {
                    _warningLabel->setText(tr("This folder is not compatible with Lite Sync."
                                              " Please select another folder or if you continue Lite Sync will be disabled."
                                              " <a style=\"%1\" href=\"ref\">Learn more</a>")
                                           .arg(OCC::Utility::linkStyle));
                    _warningWidget->setVisible(true);
                }
                else {
                    _warningWidget->setVisible(false);
                }
            }
        }
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

void AddDriveLocalFolderWidget::setInfoIcon()
{
    if (_infoIconColor != QColor() && _infoIconSize != QSize()) {
        _infoIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/information.svg", _infoIconColor)
                                   .pixmap(_infoIconSize));
    }
}

void AddDriveLocalFolderWidget::setWarningIcon()
{
    if (_warningIconColor != QColor() && _warningIconSize != QSize()) {
        _warningIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/warning.svg", _warningIconColor)
                                   .pixmap(_warningIconSize));
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
}

}

