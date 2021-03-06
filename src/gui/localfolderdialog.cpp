/*
Infomaniak Drive
Copyright (C) 2021 christophe.larchier@infomaniak.com

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

#include "localfolderdialog.h"
#include "customtoolbutton.h"
#include "guiutility.h"
#include "common/utility.h"
#include "common/vfs.h"
#include "configfile.h"
#include "custommessagebox.h"
#include "config.h"

#include <QBoxLayout>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QLabel>

namespace KDC {

static const int boxHMargin = 40;
static const int titleBoxVMargin = 14;
static const int descriptionBoxVMargin = 35;
static const int folderSelectionBoxVMargin = 30;
static const int selectionBoxHMargin = 15;
static const int selectionBoxVMargin = 20;
static const int selectionBoxSpacing = 10;
static const int warningBoxSpacing = 10;

Q_LOGGING_CATEGORY(lcLocalFolderDialog, "gui.localfolderdialog", QtInfoMsg)

LocalFolderDialog::LocalFolderDialog(const QString &localFolderPath, QWidget *parent)
    : CustomDialog(true, parent)
    , _localFolderPath(localFolderPath)
    , _continueButton(nullptr)
    , _folderSelectionWidget(nullptr)
    , _folderSelectedWidget(nullptr)
    , _folderIconLabel(nullptr)
    , _folderNameLabel(nullptr)
    , _folderPathLabel(nullptr)
    , _folderIconColor(QColor())
    , _folderIconSize(QSize())
    , _warningIconColor(QColor())
    , _warningIconSize(QSize())
    , _warningWidget(nullptr)
    , _warningIconLabel(nullptr)
    , _warningLabel(nullptr)
    , _okToContinue(false)
    , _smartSync(false)
    , _folderCompatibleWithSmartSync(false)
{
    initUI();
    updateUI();
}

void LocalFolderDialog::initUI()
{
    QVBoxLayout *mainLayout = this->mainLayout();

    // Title
    QLabel *titleLabel = new QLabel(this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    titleLabel->setText(tr("Which folder on your computer would you like to<br>synchronize ?"));
    titleLabel->setWordWrap(true);
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(titleBoxVMargin);

    // Description
    QLabel *descriptionLabel = new QLabel(this);
    descriptionLabel->setObjectName("descriptionLabel");
    descriptionLabel->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    descriptionLabel->setText(tr("The content of this folder will be synchronized on the kDrive"));
    mainLayout->addWidget(descriptionLabel);
    mainLayout->addSpacing(descriptionBoxVMargin);

    QVBoxLayout *folderSelectionVBox = new QVBoxLayout();
    folderSelectionVBox->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    mainLayout->addLayout(folderSelectionVBox);
    mainLayout->addSpacing(folderSelectionBoxVMargin);

    // Folder selection widget
    _folderSelectionWidget = new QWidget(this);
    _folderSelectionWidget->setObjectName("folderSelectionWidget");
    folderSelectionVBox->addWidget(_folderSelectionWidget);

    QHBoxLayout *folderSelectionButtonHBox = new QHBoxLayout();
    folderSelectionButtonHBox->setContentsMargins(selectionBoxHMargin, selectionBoxVMargin, selectionBoxHMargin, selectionBoxVMargin);
    _folderSelectionWidget->setLayout(folderSelectionButtonHBox);

    QPushButton *selectButton = new QPushButton(tr("Select a folder"), this);
    selectButton->setObjectName("selectButton");
    selectButton->setFlat(true);
    folderSelectionButtonHBox->addWidget(selectButton);
    folderSelectionButtonHBox->addStretch();

    // Folder selected widget
    _folderSelectedWidget = new QWidget(this);
    _folderSelectedWidget->setObjectName("folderSelectionWidget");
    folderSelectionVBox->addWidget(_folderSelectedWidget);

    QHBoxLayout *folderSelectedHBox = new QHBoxLayout();
    folderSelectedHBox->setContentsMargins(selectionBoxHMargin, selectionBoxVMargin, selectionBoxHMargin, selectionBoxVMargin);
    _folderSelectedWidget->setLayout(folderSelectedHBox);

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
    _folderPathLabel->setContextMenuPolicy(Qt::PreventContextMenu);
    folderSelectedVBox->addWidget(_folderPathLabel);

    CustomToolButton *updateButton = new CustomToolButton(this);
    updateButton->setIconPath(":/client/resources/icons/actions/edit.svg");
    updateButton->setToolTip(tr("Edit folder"));
    folderSelectedHBox->addWidget(updateButton);

    // Warning
    _warningWidget = new QWidget(this);
    _warningWidget->setVisible(false);
    mainLayout->addWidget(_warningWidget);
    mainLayout->addStretch();

    QVBoxLayout *warningVBox = new QVBoxLayout();
    warningVBox->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    _warningWidget->setLayout(warningVBox);

    QHBoxLayout *warningHBox = new QHBoxLayout();
    warningHBox->setContentsMargins(0, 0, 0, 0);
    warningHBox->setSpacing(warningBoxSpacing);
    warningVBox->addLayout(warningHBox);

    _warningIconLabel = new QLabel(this);
    warningHBox->addWidget(_warningIconLabel);

    _warningLabel = new QLabel(this);
    _warningLabel->setObjectName("largeMediumTextLabel");
    _warningLabel->setWordWrap(true);
    warningHBox->addWidget(_warningLabel);
    warningHBox->setStretchFactor(_warningLabel, 1);

    // Add dialog buttons
    QHBoxLayout *buttonsHBox = new QHBoxLayout();
    buttonsHBox->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    mainLayout->addLayout(buttonsHBox);

    QPushButton *cancelButton = new QPushButton(this);
    cancelButton->setObjectName("nondefaultbutton");
    cancelButton->setFlat(true);
    cancelButton->setText(tr("CANCEL"));
    buttonsHBox->addWidget(cancelButton);
    buttonsHBox->addStretch();

    _continueButton = new QPushButton(this);
    _continueButton->setObjectName("defaultbutton");
    _continueButton->setFlat(true);
    _continueButton->setText(tr("CONTINUE"));
    _continueButton->setEnabled(false);
    buttonsHBox->addWidget(_continueButton);

    connect(selectButton, &QPushButton::clicked, this, &LocalFolderDialog::onSelectFolderButtonTriggered);
    connect(updateButton, &CustomToolButton::clicked, this, &LocalFolderDialog::onUpdateFolderButtonTriggered);
    connect(_folderPathLabel, &QLabel::linkActivated, this, &LocalFolderDialog::onLinkActivated);
    connect(_continueButton, &QPushButton::clicked, this, &LocalFolderDialog::onContinueButtonTriggered);
    connect(cancelButton, &QPushButton::clicked, this, &LocalFolderDialog::onExit);
    connect(this, &CustomDialog::exit, this, &LocalFolderDialog::onExit);
}

void LocalFolderDialog::updateUI()
{
    bool ok = !_localFolderPath.isEmpty();
    if (ok) {
        QDir dir(_localFolderPath);
        _folderNameLabel->setText(dir.dirName());
        _folderPathLabel->setText(QString("<a style=\"%1\" href=\"ref\">%2</a>")
                                  .arg(OCC::Utility::linkStyle)
                                  .arg(_localFolderPath));
    }
    _folderSelectionWidget->setVisible(!ok);
    _folderSelectedWidget->setVisible(ok);

    if (_smartSync) {
        OCC::Vfs::Mode mode = OCC::bestAvailableVfsMode(OCC::ConfigFile().showExperimentalOptions());
        if (mode == OCC::Vfs::WindowsCfApi) {
            // Check file system
            QString fsName(OCC::Utility::fileSystemName(_localFolderPath));
            _folderCompatibleWithSmartSync = (fsName == "NTFS" && !QDir(_localFolderPath).isRoot());
            if (!_folderCompatibleWithSmartSync) {
                _warningLabel->setText(tr("This folder is not compatible with Lite Sync."
                                          " Please select another folder or if you continue Lite Sync will be disabled."
                                          " <a style=\"%1\" href=\"%2\">Learn more</a>")
                                       .arg(OCC::Utility::linkStyle)
                                       .arg(OCC::Utility::learnMoreLink));
                _warningWidget->setVisible(true);
            }
            else {
                _warningWidget->setVisible(false);
            }
        }
    }

    setOkToContinue(ok);
    forceRedraw();
}

void LocalFolderDialog::setOkToContinue(bool value)
{
    _okToContinue = value;
    _continueButton->setEnabled(value);
}

void LocalFolderDialog::selectFolder(const QString &startDirPath)
{
    QString dirPath = QFileDialog::getExistingDirectory(this, tr("Select folder"), startDirPath,
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dirPath.isEmpty()) {
        QDir dir(dirPath);
        _localFolderPath = dir.canonicalPath();
        updateUI();
    }
}

void LocalFolderDialog::setFolderIcon()
{
    if (_folderIconColor != QColor() && _folderIconSize != QSize()) {
        _folderIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/folder.svg", _folderIconColor)
                                   .pixmap(_folderIconSize));
    }
}

void LocalFolderDialog::setWarningIcon()
{
    if (_warningIconColor != QColor() && _warningIconSize != QSize()) {
        _warningIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/warning.svg", _warningIconColor)
                                   .pixmap(_warningIconSize));
    }
}

void LocalFolderDialog::onExit()
{
    reject();
}

void LocalFolderDialog::onContinueButtonTriggered(bool checked)
{
    Q_UNUSED(checked)

    accept();
}

void LocalFolderDialog::onSelectFolderButtonTriggered(bool checked)
{
    Q_UNUSED(checked)

    selectFolder(QDir::homePath());
}

void LocalFolderDialog::onUpdateFolderButtonTriggered(bool checked)
{
    Q_UNUSED(checked)

    selectFolder(_localFolderPath);
}

void LocalFolderDialog::onLinkActivated(const QString &link)
{
    if (link == OCC::Utility::learnMoreLink) {
        // Learn more: Folder not compatible with Lite Sync
        if (!QDesktopServices::openUrl(QUrl(LEARNMORE_LITESYNC_COMPATIBILITY_URL))) {
            qCWarning(lcLocalFolderDialog) << "QDesktopServices::openUrl failed for " << link;
            CustomMessageBox *msgBox = new CustomMessageBox(
                        QMessageBox::Warning,
                        tr("Unable to open link %1.").arg(link),
                        QMessageBox::Ok, this);
            msgBox->exec();
        }
    }
    else {
        emit openFolder(_localFolderPath);
    }
}

}
