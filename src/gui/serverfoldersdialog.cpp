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

#include "serverfoldersdialog.h"
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

Q_LOGGING_CATEGORY(lcServerFoldersDialog, "serverfoldersdialog", QtInfoMsg)

ServerFoldersDialog::ServerFoldersDialog(const AccountInfo *accountInfo, QWidget *parent)
    : CustomDialog(true, parent)
    , _accountInfo(accountInfo)
    , _infoIconLabel(nullptr)
    , _availableSpaceTextLabel(nullptr)
    , _messageLabel(nullptr)
    , _folderTreeItemWidget(nullptr)
    , _saveButton(nullptr)
    , _infoIconColor(QColor())
    , _infoIconSize(QSize())
    , _needToSave(false)
{
    initUI();
    updateUI();
}

void ServerFoldersDialog::setInfoIcon()
{
    if (_infoIconLabel && _infoIconSize != QSize() && _infoIconColor != QColor()) {
        _infoIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/information.svg",
                                                                 _infoIconColor).pixmap(_infoIconSize));
    }
}

void ServerFoldersDialog::initUI()
{
    QVBoxLayout *mainLayout = this->mainLayout();

    // Title
    QLabel *titleLabel = new QLabel(this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    titleLabel->setText(tr("kDrive folders"));
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(titleBoxVMargin);

    // Description
    QLabel *descriptionLabel = new QLabel(this);
    descriptionLabel->setObjectName("descriptionLabel");
    descriptionLabel->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    descriptionLabel->setText(tr("Select the folders you want to synchronize on your computer."));
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

    QString folderId = _accountInfo->_folderMap.begin()->first;
    _currentFolder = OCC::FolderMan::instance()->folder(folderId);
    if (!_currentFolder) {
        qCDebug(lcServerFoldersDialog) << "Folder not found: " << folderId;
    }

    _folderTreeItemWidget = new FolderTreeItemWidget(folderId, true, this);
    folderTreeHBox->addWidget(_folderTreeItemWidget);
    mainLayout->setStretchFactor(_folderTreeItemWidget, 1);

    // Add dialog buttons
    QHBoxLayout *buttonsHBox = new QHBoxLayout();
    buttonsHBox->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    buttonsHBox->setSpacing(boxHSpacing);
    mainLayout->addLayout(buttonsHBox);

    _saveButton = new QPushButton(this);
    _saveButton->setObjectName("defaultbutton");
    _saveButton->setFlat(true);
    _saveButton->setText(tr("SAVE"));
    _saveButton->setEnabled(false);
    buttonsHBox->addWidget(_saveButton);

    QPushButton *cancelButton = new QPushButton(this);
    cancelButton->setObjectName("nondefaultbutton");
    cancelButton->setFlat(true);
    cancelButton->setText(tr("CANCEL"));
    buttonsHBox->addWidget(cancelButton);
    buttonsHBox->addStretch();

    connect(_folderTreeItemWidget, &FolderTreeItemWidget::message, this, &ServerFoldersDialog::onDisplayMessage);
    connect(_folderTreeItemWidget, &FolderTreeItemWidget::showMessage, this, &ServerFoldersDialog::onShowMessage);
    connect(_folderTreeItemWidget, &FolderTreeItemWidget::needToSave, this, &ServerFoldersDialog::onNeedToSave);
    connect(_saveButton, &QPushButton::clicked, this, &ServerFoldersDialog::onSaveButtonTriggered);
    connect(cancelButton, &QPushButton::clicked, this, &ServerFoldersDialog::onExit);
    connect(this, &CustomDialog::exit, this, &ServerFoldersDialog::onExit);
}

void ServerFoldersDialog::updateUI()
{
    // Available space
    qint64 freeBytes = OCC::Utility::freeDiskSpace(_currentFolder->remotePath());
    _availableSpaceTextLabel->setText(tr("Space available on your computer for the current folder : %1")
                                      .arg(OCC::Utility::octetsToString(freeBytes)));

    _folderTreeItemWidget->clear();
    onShowMessage(true);
}

void ServerFoldersDialog::setNeedToSave(bool value)
{
    _needToSave = value;
    _saveButton->setEnabled(value);
}

void ServerFoldersDialog::onInfoIconSizeChanged()
{
    setInfoIcon();
}

void ServerFoldersDialog::onInfoIconColorChanged()
{
    setInfoIcon();
}

void ServerFoldersDialog::onExit()
{
    if (_needToSave) {
        CustomMessageBox *msgBox = new CustomMessageBox(
                    QMessageBox::Question,
                    tr("Do you want to save your modifications?"),
                    QMessageBox::Yes | QMessageBox::No, this);
        msgBox->setDefaultButton(QMessageBox::Yes);
        int ret = msgBox->exec();
        if (ret != QDialog::Rejected) {
            if (ret == QMessageBox::Yes) {
                onSaveButtonTriggered();
            }
            else {
                reject();
            }
        }
    }
    else {
        reject();
    }
}

void ServerFoldersDialog::onSaveButtonTriggered(bool checked)
{
    Q_UNUSED(checked)

    bool ok;
    auto oldBlackListSet = _currentFolder->journalDb()->getSelectiveSyncList(OCC::SyncJournalDb::SelectiveSyncBlackList, &ok).toSet();
    if (!ok) {
        return;
    }

    QStringList blackList = _folderTreeItemWidget->createBlackList();
    _currentFolder->journalDb()->setSelectiveSyncList(OCC::SyncJournalDb::SelectiveSyncBlackList, blackList);

    if (_currentFolder->isBusy()) {
        _currentFolder->slotTerminateSync();
    }

    // The part that changed should not be read from the DB on next sync because there might be new folders
    // (the ones that are no longer in the blacklist)
    auto blackListSet = blackList.toSet();
    auto changes = (oldBlackListSet - blackListSet) + (blackListSet - oldBlackListSet);
    foreach (const auto &it, changes) {
        _currentFolder->journalDb()->schedulePathForRemoteDiscovery(it);
        _currentFolder->schedulePathForLocalDiscovery(it);
    }
    // Also make sure we see the local file that had been ignored before
    _currentFolder->slotNextSyncFullLocalDiscovery();

    OCC::FolderMan::instance()->scheduleFolder(_currentFolder);

    accept();
}

void ServerFoldersDialog::onDisplayMessage(const QString &text)
{
    _messageLabel->setText(text);
}

void ServerFoldersDialog::onShowMessage(bool show)
{
    _messageLabel->setVisible(show);
}

void ServerFoldersDialog::onNeedToSave()
{
    setNeedToSave(true);
}

}

