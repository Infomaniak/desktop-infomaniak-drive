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

#include "serverbasefolderdialog.h"
#include "custommessagebox.h"
#include "guiutility.h"

#include <QBoxLayout>

namespace KDC {

static const int boxHMargin = 40;
static const int boxHSpacing = 10;
static const int titleBoxVMargin = 14;
static const int descriptionBoxVMargin = 15;
static const int availableSpaceBoxVMargin = 20;
static const int messageVMargin = 20;
static const int folderTreeBoxVMargin = 20;

Q_LOGGING_CATEGORY(lcServerBaseFolderDialog, "serverbasefolderdialog", QtInfoMsg)

ServerBaseFolderDialog::ServerBaseFolderDialog(const QString &accountId, const QString &localFolderName, QWidget *parent)
    : CustomDialog(true, parent)
    , _accountId(accountId)
    , _localFolderName(localFolderName)
    , _infoIconLabel(nullptr)
    , _availableSpaceTextLabel(nullptr)
    , _messageLabel(nullptr)
    , _folderTreeItemWidget(nullptr)
    , _continueButton(nullptr)
    , _infoIconColor(QColor())
    , _infoIconSize(QSize())
    , _okToContinue(false)
{
    initUI();
    updateUI();
}

void ServerBaseFolderDialog::setInfoIcon()
{
    if (_infoIconLabel && _infoIconSize != QSize() && _infoIconColor != QColor()) {
        _infoIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/information.svg",
                                                                 _infoIconColor).pixmap(_infoIconSize));
    }
}

void ServerBaseFolderDialog::initUI()
{
    QVBoxLayout *mainLayout = this->mainLayout();

    // Title
    QLabel *titleLabel = new QLabel(this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    titleLabel->setText(tr("Select a folder on your kDrive"));
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(titleBoxVMargin);

    // Description
    QLabel *descriptionLabel = new QLabel(this);
    descriptionLabel->setObjectName("descriptionLabel");
    descriptionLabel->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    descriptionLabel->setText(tr("The content of the selected folder will be synchronized into the <b>%1</b> folder.")
                              .arg(_localFolderName));
    mainLayout->addWidget(descriptionLabel);
    mainLayout->addSpacing(descriptionBoxVMargin);

    // Available space
    QHBoxLayout *availableSpaceHBox = new QHBoxLayout();
    availableSpaceHBox->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    availableSpaceHBox->setSpacing(boxHSpacing);
    mainLayout->addLayout(availableSpaceHBox);
    mainLayout->addSpacing(availableSpaceBoxVMargin);

    _infoIconLabel = new QLabel(this);
    availableSpaceHBox->addWidget(_infoIconLabel);

    _availableSpaceTextLabel = new QLabel(this);
    _availableSpaceTextLabel->setObjectName("largeMediumTextLabel");
    availableSpaceHBox->addWidget(_availableSpaceTextLabel);
    availableSpaceHBox->addStretch();

    // Message
    _messageLabel = new QLabel(this);
    _messageLabel->setObjectName("messageLabel");
    _messageLabel->setText(tr("Loading..."));
    _messageLabel->setVisible(false);
    _messageLabel->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    mainLayout->addWidget(_messageLabel);
    mainLayout->addSpacing(messageVMargin);

    // Folder tree
    QHBoxLayout *folderTreeHBox = new QHBoxLayout();
    folderTreeHBox->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    mainLayout->addLayout(folderTreeHBox);
    mainLayout->addSpacing(folderTreeBoxVMargin);

    _folderTreeItemWidget = new BaseFolderTreeItemWidget(_accountId, true, this);
    folderTreeHBox->addWidget(_folderTreeItemWidget);
    mainLayout->setStretchFactor(_folderTreeItemWidget, 1);

    // Add dialog buttons
    QHBoxLayout *buttonsHBox = new QHBoxLayout();
    buttonsHBox->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    buttonsHBox->setSpacing(boxHSpacing);
    mainLayout->addLayout(buttonsHBox);

    QPushButton *backButton = new QPushButton(this);
    backButton->setObjectName("nondefaultbutton");
    backButton->setFlat(true);
    backButton->setIcon(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/chevron-left.svg"));
    buttonsHBox->addWidget(backButton);
    buttonsHBox->addStretch();

    _continueButton = new QPushButton(this);
    _continueButton->setObjectName("defaultbutton");
    _continueButton->setFlat(true);
    _continueButton->setText(tr("CONTINUE"));
    _continueButton->setEnabled(false);
    buttonsHBox->addWidget(_continueButton);

    connect(_folderTreeItemWidget, &BaseFolderTreeItemWidget::message, this, &ServerBaseFolderDialog::onDisplayMessage);
    connect(_folderTreeItemWidget, &BaseFolderTreeItemWidget::showMessage, this, &ServerBaseFolderDialog::onShowMessage);
    connect(_folderTreeItemWidget, &BaseFolderTreeItemWidget::folderSelected, this, &ServerBaseFolderDialog::onFolderSelected);
    connect(backButton, &QPushButton::clicked, this, &ServerBaseFolderDialog::onBackButtonTriggered);
    connect(_continueButton, &QPushButton::clicked, this, &ServerBaseFolderDialog::onContinueButtonTriggered);
    connect(this, &CustomDialog::exit, this, &ServerBaseFolderDialog::onExit);
}

void ServerBaseFolderDialog::updateUI()
{
    // Available space
    qint64 freeBytes = OCC::Utility::freeDiskSpace(QString("/"));
    _availableSpaceTextLabel->setText(tr("Space available on your computer for the current folder : %1")
                                      .arg(OCC::Utility::octetsToString(freeBytes)));

    _folderTreeItemWidget->loadSubFolders();
    onShowMessage(true);
}

void ServerBaseFolderDialog::setOkToContinue(bool value)
{
    _okToContinue = value;
    _continueButton->setEnabled(value);
}

void ServerBaseFolderDialog::onInfoIconSizeChanged()
{
    setInfoIcon();
}

void ServerBaseFolderDialog::onInfoIconColorChanged()
{
    setInfoIcon();
}

void ServerBaseFolderDialog::onExit()
{
    reject();
}

void ServerBaseFolderDialog::onContinueButtonTriggered(bool checked)
{
    Q_UNUSED(checked)

    accept();
}

void ServerBaseFolderDialog::onBackButtonTriggered(bool checked)
{
    Q_UNUSED(checked)

    done(-1);
}

void ServerBaseFolderDialog::onDisplayMessage(const QString &text)
{
    _messageLabel->setText(text);
}

void ServerBaseFolderDialog::onShowMessage(bool show)
{
    _messageLabel->setVisible(show);
}

void ServerBaseFolderDialog::onFolderSelected(const QString &folderPath)
{
    _serverFolderPath = folderPath;
    setOkToContinue(true);
}

}

