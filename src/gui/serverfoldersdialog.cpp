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
#include <QDir>

namespace KDC {

static const int boxHMargin = 40;
static const int boxHSpacing = 10;
static const int titleBoxVMargin = 14;
static const int folderTreeBoxVMargin = 20;

Q_LOGGING_CATEGORY(lcServerFoldersDialog, "serverfoldersdialog", QtInfoMsg)

ServerFoldersDialog::ServerFoldersDialog(const QString &accountId, const QString &serverFolderName,
                                         const QString &serverFolderPath, QWidget *parent)
    : CustomDialog(true, parent)
    , _accountId(accountId)
    , _serverFolderName(serverFolderName)
    , _serverFolderPath(serverFolderPath)
    , _folderTreeItemWidget(nullptr)
    , _backButton(nullptr)
    , _continueButton(nullptr)
    , _needToSave(false)
{
    initUI();
    updateUI();
}

qint64 ServerFoldersDialog::selectionSize() const
{
    CustomTreeWidgetItem *root = static_cast<CustomTreeWidgetItem *>(_folderTreeItemWidget->topLevelItem(0));
    if (root) {
        return _folderTreeItemWidget->nodeSize(root);
    }
    return 0;
}

QStringList ServerFoldersDialog::createBlackList() const
{
    return _folderTreeItemWidget->createBlackList();
}

void ServerFoldersDialog::setButtonIcon(const QColor &value)
{
    _backButton->setIcon(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/chevron-left.svg", value));
}

void ServerFoldersDialog::initUI()
{
    QVBoxLayout *mainLayout = this->mainLayout();

    // Title
    QLabel *titleLabel = new QLabel(this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    titleLabel->setText(tr("The <b>%1</b> folder contains subfolders,<br> select the ones you want to synchronize")
                        .arg(_serverFolderName));
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(titleBoxVMargin);

    // Folder tree
    QHBoxLayout *folderTreeHBox = new QHBoxLayout();
    folderTreeHBox->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    mainLayout->addLayout(folderTreeHBox);
    mainLayout->addSpacing(folderTreeBoxVMargin);

    _folderTreeItemWidget = new FolderTreeItemWidget(_accountId, _serverFolderPath, true, this);
    folderTreeHBox->addWidget(_folderTreeItemWidget);
    mainLayout->setStretchFactor(_folderTreeItemWidget, 1);

    // Add dialog buttons
    QHBoxLayout *buttonsHBox = new QHBoxLayout();
    buttonsHBox->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
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

    connect(_folderTreeItemWidget, &FolderTreeItemWidget::terminated, this, &ServerFoldersDialog::onSubfoldersLoaded);
    connect(_folderTreeItemWidget, &FolderTreeItemWidget::needToSave, this, &ServerFoldersDialog::onNeedToSave);
    connect(_backButton, &QPushButton::clicked, this, &ServerFoldersDialog::onBackButtonTriggered);
    connect(_continueButton, &QPushButton::clicked, this, &ServerFoldersDialog::onContinueButtonTriggered);
    connect(this, &CustomDialog::exit, this, &ServerFoldersDialog::onExit);
}

void ServerFoldersDialog::updateUI()
{
    _folderTreeItemWidget->loadSubFolders();
}

void ServerFoldersDialog::onExit()
{
    reject();
}

void ServerFoldersDialog::onBackButtonTriggered(bool checked)
{
    Q_UNUSED(checked)

    done(-1);
}

void ServerFoldersDialog::onContinueButtonTriggered(bool checked)
{
    Q_UNUSED(checked)

    accept();
}

void ServerFoldersDialog::onSubfoldersLoaded(bool error, bool empty)
{
    FolderTreeItemWidget *folderTreeItemWidget = qobject_cast<FolderTreeItemWidget *>(sender());
    folderTreeItemWidget->setVisible(!error && !empty);
    if (error) {
        CustomMessageBox *msgBox = new CustomMessageBox(
                    QMessageBox::Warning,
                    tr("An error occurred while loading the list of sub folders."),
                    QMessageBox::Ok, this);
        msgBox->setDefaultButton(QMessageBox::Ok);
        msgBox->exec();
        reject();
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

void ServerFoldersDialog::onNeedToSave()
{
    _needToSave = true;
}

}

