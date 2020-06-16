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

#include "adddriveserverfolderswidget.h"
#include "custommessagebox.h"
#include "guiutility.h"
#include "wizard/owncloudwizardcommon.h"

#include <QBoxLayout>
#include <QDir>
#include <QProgressBar>

namespace KDC {

static const int boxHSpacing = 10;
static const int logoBoxVMargin = 20;
static const int progressBarVMargin = 40;
static const int hLogoSpacing = 20;
static const int logoIconSize = 39;
static const QSize logoTextIconSize = QSize(60, 42);
static const int titleBoxVMargin = 20;
static const int availableSpaceBoxVMargin = 20;
static const int folderTreeBoxVMargin = 20;
static const int progressBarMin = 0;
static const int progressBarMax = 4;

Q_LOGGING_CATEGORY(lcAddDriveServerFoldersWidget, "adddriveserverfolderswidget", QtInfoMsg)

AddDriveServerFoldersWidget::AddDriveServerFoldersWidget(QWidget *parent)
    : QWidget(parent)
    , _currentFolder(nullptr)
    , _logoTextIconLabel(nullptr)
    , _infoIconLabel(nullptr)
    , _availableSpaceTextLabel(nullptr)
    , _folderTreeItemWidget(nullptr)
    , _backButton(nullptr)
    , _continueButton(nullptr)
    , _infoIconColor(QColor())
    , _infoIconSize(QSize())
    , _logoColor(QColor())
    , _needToSave(false)
{
    initUI();
    updateUI();
}

void AddDriveServerFoldersWidget::setAccountPtr(OCC::AccountPtr accountPtr)
{
    _folderTreeItemWidget->setAccountPtr(accountPtr);
    _folderTreeItemWidget->loadSubFolders();
}

qint64 AddDriveServerFoldersWidget::selectionSize() const
{
    CustomTreeWidgetItem *root = static_cast<CustomTreeWidgetItem *>(_folderTreeItemWidget->topLevelItem(0));
    if (root) {
        return _folderTreeItemWidget->nodeSize(root);
    }
    return 0;
}

QStringList AddDriveServerFoldersWidget::createBlackList() const
{
    return _folderTreeItemWidget->createBlackList();
}

void AddDriveServerFoldersWidget::setButtonIcon(const QColor &value)
{
    if (_backButton) {
        _backButton->setIcon(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/chevron-left.svg", value));
    }
}

void AddDriveServerFoldersWidget::initUI()
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
    progressBar->setValue(2);
    progressBar->setFormat(QString());
    mainLayout->addWidget(progressBar);
    mainLayout->addSpacing(progressBarVMargin);

    // Title
    QLabel *titleLabel = new QLabel(this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setContentsMargins(0, 0, 0, 0);
    titleLabel->setText(tr("Select kDrive folders to synchronize on your desktop"));
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(titleBoxVMargin);

    // Available space
    QHBoxLayout *availableSpaceHBox = new QHBoxLayout();
    availableSpaceHBox->setContentsMargins(0, 0, 0, 0);
    availableSpaceHBox->setSpacing(boxHSpacing);
    mainLayout->addLayout(availableSpaceHBox);
    mainLayout->addSpacing(availableSpaceBoxVMargin);

    _infoIconLabel = new QLabel(this);
    availableSpaceHBox->addWidget(_infoIconLabel);

    _availableSpaceTextLabel = new QLabel(this);
    _availableSpaceTextLabel->setObjectName("largeMediumTextLabel");
    availableSpaceHBox->addWidget(_availableSpaceTextLabel);
    availableSpaceHBox->addStretch();

    // Folder tree
    QHBoxLayout *folderTreeHBox = new QHBoxLayout();
    folderTreeHBox->setContentsMargins(0, 0, 0, 0);
    mainLayout->addLayout(folderTreeHBox);
    mainLayout->addSpacing(folderTreeBoxVMargin);

    _folderTreeItemWidget = new FolderTreeItemWidget(QString(), QString(), true, this);
    folderTreeHBox->addWidget(_folderTreeItemWidget);
    mainLayout->setStretchFactor(_folderTreeItemWidget, 1);

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

    _continueButton = new QPushButton(this);
    _continueButton->setObjectName("defaultbutton");
    _continueButton->setFlat(true);
    _continueButton->setText(tr("CONTINUE"));
    buttonsHBox->addWidget(_continueButton);

    connect(_folderTreeItemWidget, &FolderTreeItemWidget::terminated, this, &AddDriveServerFoldersWidget::onSubfoldersLoaded);
    connect(_folderTreeItemWidget, &FolderTreeItemWidget::needToSave, this, &AddDriveServerFoldersWidget::onNeedToSave);
    connect(_backButton, &QPushButton::clicked, this, &AddDriveServerFoldersWidget::onBackButtonTriggered);
    connect(_continueButton, &QPushButton::clicked, this, &AddDriveServerFoldersWidget::onContinueButtonTriggered);
}

void AddDriveServerFoldersWidget::updateUI()
{
    // Available space
    qint64 freeBytes = OCC::Utility::freeDiskSpace(dirSeparator);
    _availableSpaceTextLabel->setText(tr("Space available on your computer : %1")
                                      .arg(OCC::Utility::octetsToString(freeBytes)));
}

void AddDriveServerFoldersWidget::setInfoIcon()
{
    if (_infoIconLabel && _infoIconSize != QSize() && _infoIconColor != QColor()) {
        _infoIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/information.svg",
                                                                 _infoIconColor).pixmap(_infoIconSize));
    }
}

void AddDriveServerFoldersWidget::setLogoColor(const QColor &color)
{
    _logoColor = color;
    _logoTextIconLabel->setPixmap(
                OCC::Utility::getIconWithColor(":/client/resources/logos/kdrive-text-only.svg", _logoColor)
                .pixmap(logoTextIconSize));
}

void AddDriveServerFoldersWidget::onSubfoldersLoaded(bool error, bool empty)
{
    if (error) {
        CustomMessageBox *msgBox = new CustomMessageBox(
                    QMessageBox::Warning,
                    tr("An error occurred while loading the list of sub folders."),
                    QMessageBox::Ok, this);
        msgBox->setDefaultButton(QMessageBox::Ok);
        msgBox->exec();
        emit terminated(false);
    }
    else if (empty) {
        CustomMessageBox *msgBox = new CustomMessageBox(
                    QMessageBox::Warning,
                    tr("No subfolders currently on the server."),
                    QMessageBox::Ok, this);
        msgBox->setDefaultButton(QMessageBox::Ok);
        msgBox->exec();
    }
}

void AddDriveServerFoldersWidget::onNeedToSave()
{
    _needToSave = true;
}

void AddDriveServerFoldersWidget::onBackButtonTriggered(bool checked)
{
    Q_UNUSED(checked)

    emit terminated(false);
}

void AddDriveServerFoldersWidget::onContinueButtonTriggered(bool checked)
{
    Q_UNUSED(checked)

    emit terminated();
}

}

