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

ServerFoldersDialog::ServerFoldersDialog(const QString &accountId, const QString &serverFolderPath, QWidget *parent)
    : CustomDialog(true, parent)
    , _accountId(accountId)
    , _serverFolderPath(serverFolderPath)
    , _folderTreeItemWidget(nullptr)
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
        return _folderTreeItemWidget->selectionSize(root);
    }
    return 0;
}

QStringList ServerFoldersDialog::createBlackList() const
{
    return _folderTreeItemWidget->createBlackList();
}

void ServerFoldersDialog::initUI()
{
    QVBoxLayout *mainLayout = this->mainLayout();

    // Title
    QLabel *titleLabel = new QLabel(this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    QDir dir(_serverFolderPath);
    titleLabel->setText(tr("The <b>%1</b> folder contains subfolders,<br> select the ones you want to synchronize")
                        .arg(dir.dirName()));
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
    buttonsHBox->addWidget(_continueButton);

    connect(_folderTreeItemWidget, &FolderTreeItemWidget::message, this, &ServerFoldersDialog::onDisplayMessage);
    connect(_folderTreeItemWidget, &FolderTreeItemWidget::needToSave, this, &ServerFoldersDialog::onNeedToSave);
    connect(backButton, &QPushButton::clicked, this, &ServerFoldersDialog::onBackButtonTriggered);
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

void ServerFoldersDialog::onDisplayMessage(const QString &text)
{
    CustomMessageBox *msgBox = new CustomMessageBox(
                QMessageBox::Warning,
                text,
                QMessageBox::Ok, this);
    msgBox->setDefaultButton(QMessageBox::Ok);
    msgBox->exec();
}

void ServerFoldersDialog::onNeedToSave()
{
    _needToSave = true;
}

}

