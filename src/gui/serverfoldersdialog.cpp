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
#include "networkjobs.h"
#include "folderman.h"
#include "guiutility.h"
#include "common/utility.h"

#include <QBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QMutableListIterator>
#include <QScopedValueRollback>

namespace KDC {

static const int boxHMargin= 40;
static const int boxHSpacing = 10;
static const int titleBoxVMargin = 14;
static const int descriptionBoxVMargin = 20;
static const int availableSpaceBoxVMargin = 45;
static const int folderTreeBoxVMargin= 20;

ServerFoldersDialog::ServerFoldersDialog(const AccountInfo *accountInfo, QWidget *parent)
    : CustomDialog(true, parent)
    , _accountInfo(accountInfo)
    , _currentFolderId(QString())
    , _currentFolderPath(QString())
    , _inserting(false)
    , _infoIconLabel(nullptr)
    , _availableSpaceTextLabel(nullptr)
    , _folderTreeWidget(nullptr)
    , _saveButton(nullptr)
    , _infoIconColor(QColor())
    , _infoIconSize(QSize())
    , _needToSave(false)
{
    setStyleSheet("QTreeWidget::indicator:checked { image: url(:/client/resources/icons/actions/checkbox-checked.svg); }"
                  "QTreeWidget::indicator:unchecked { image: url(:/client/resources/icons/actions/checkbox-unchecked.svg); }"
                  "QTreeWidget::indicator:checked:disabled { image: url(:/client/resources/icons/actions/checkbox-checked.svg); }"
                  "QTreeWidget::indicator:unchecked:disabled { image: url(:/client/resources/icons/actions/checkbox-unchecked.svg); }");

    initUI();
    updateUI();

    connect(this, &ServerFoldersDialog::infoIconSizeChanged, this, &ServerFoldersDialog::onInfoIconSizeChanged);
    connect(this, &ServerFoldersDialog::infoIconColorChanged, this, &ServerFoldersDialog::onInfoIconColorChanged);
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
    titleLabel->setContentsMargins(boxHMargin, 0, boxHMargin, titleBoxVMargin);
    titleLabel->setText(tr("kDrive folders"));
    mainLayout->addWidget(titleLabel);

    // Description
    QLabel *descriptionLabel = new QLabel(this);
    descriptionLabel->setObjectName("descriptionLabel");
    descriptionLabel->setContentsMargins(boxHMargin, 0, boxHMargin, descriptionBoxVMargin);
    descriptionLabel->setText(tr("Select the folders you want to synchronize on your computer."));
    mainLayout->addWidget(descriptionLabel);

    // Available space
    QHBoxLayout *availableSpaceHBox = new QHBoxLayout();
    availableSpaceHBox->setContentsMargins(boxHMargin, 0, boxHMargin, availableSpaceBoxVMargin);
    availableSpaceHBox->setSpacing(boxHSpacing);
    mainLayout->addLayout(availableSpaceHBox);

    _infoIconLabel = new QLabel(this);
    availableSpaceHBox->addWidget(_infoIconLabel);

    _availableSpaceTextLabel = new QLabel(this);
    _availableSpaceTextLabel->setObjectName("largeTextLabel");
    availableSpaceHBox->addWidget(_availableSpaceTextLabel);
    availableSpaceHBox->addStretch();

    // Folder tree
    QHBoxLayout *folderTreeHBox = new QHBoxLayout();
    folderTreeHBox->setContentsMargins(boxHMargin, 0, boxHMargin, folderTreeBoxVMargin);
    mainLayout->addLayout(folderTreeHBox);

    _folderTreeWidget = new QTreeWidget(this);
    _folderTreeWidget->setSortingEnabled(true);
    _folderTreeWidget->sortByColumn(0, Qt::AscendingOrder);
    _folderTreeWidget->setColumnCount(2);
    _folderTreeWidget->header()->hide();
    _folderTreeWidget->header()->setSectionResizeMode(0, QHeaderView::QHeaderView::ResizeToContents);
    _folderTreeWidget->header()->setSectionResizeMode(1, QHeaderView::QHeaderView::ResizeToContents);
    _folderTreeWidget->header()->setStretchLastSection(true);
    folderTreeHBox->addWidget(_folderTreeWidget);
    mainLayout->setStretchFactor(_folderTreeWidget, 1);

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
    cancelButton->setFlat(true);
    cancelButton->setText(tr("CANCEL"));
    buttonsHBox->addWidget(cancelButton);
    buttonsHBox->addStretch();

    connect(_saveButton, &QPushButton::clicked, this, &ServerFoldersDialog::onSaveButtonTriggered);
    connect(cancelButton, &QPushButton::clicked, this, &ServerFoldersDialog::onExit);
    connect(this, &CustomDialog::exit, this, &ServerFoldersDialog::onExit);
}

void ServerFoldersDialog::updateUI()
{
    qint64 freeBytes = 0;
    for (auto folderInfoIt : _accountInfo->_folderMap) {
        freeBytes = OCC::Utility::freeDiskSpace(folderInfoIt.second->_path);
        if (_currentFolderId.isEmpty()) {
            _currentFolderId = folderInfoIt.first;
        }

        if (_currentFolderId == folderInfoIt.first) {
            _currentFolderPath = folderInfoIt.second->_path;
            break;
        }
    }

    _availableSpaceTextLabel->setText(tr("Space available on your computer for the current folder : %1")
                                      .arg(OCC::Utility::octetsToString(freeBytes)));
}

void ServerFoldersDialog::refreshFolders()
{
    OCC::LsColJob *job = new OCC::LsColJob(_account, _currentFolderPath, this);
    job->setProperties(QList<QByteArray>() << "resourcetype"
                                           << "http://owncloud.org/ns:size");
    connect(job, &OCC::LsColJob::directoryListingSubfolders, this, &ServerFoldersDialog::slotUpdateDirectories);
    connect(job, &OCC::LsColJob::finishedWithError, this, &ServerFoldersDialog::slotLscolFinishedWithError);
    job->start();
    _folderTreeWidget->clear();
    //_loading->show();
    //_loading->move(10, _folderTree->header()->height() + 10);
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
        QMessageBox msgBox(QMessageBox::Question, QString(),
                           tr("Do you want to save your modifications?"),
                           QMessageBox::Yes | QMessageBox::No, this);
        msgBox.setWindowModality(Qt::WindowModal);
        msgBox.setDefaultButton(QMessageBox::Yes);
        if (msgBox.exec() == QMessageBox::Yes) {
            onSaveButtonTriggered();
        }
        else {
            reject();
        }
    }
    else {
        reject();
    }
}

void ServerFoldersDialog::onSaveButtonTriggered(bool checked)
{



    accept();
}

void ServerFoldersDialog::slotUpdateDirectories(QStringList list)
{
    auto job = qobject_cast<OCC::LsColJob *>(sender());
    QScopedValueRollback<bool> isInserting(_inserting);
    _inserting = true;

    TreeViewItem *root = static_cast<TreeViewItem *>(_folderTreeWidget->topLevelItem(0));

    QUrl url = _account->davUrl();
    QString pathToRemove = url.path();
    if (!pathToRemove.endsWith('/')) {
        pathToRemove.append('/');
    }

    pathToRemove.append(_currentFolderPath);
    if (!_currentFolderPath.isEmpty()) {
        pathToRemove.append('/');
    }

    // Check for excludes.
    QMutableListIterator<QString> it(list);
    while (it.hasNext()) {
        it.next();
        if (_excludedFiles.isExcluded(it.value(), pathToRemove, OCC::FolderMan::instance()->ignoreHiddenFiles()))
            it.remove();
    }

    // Since / cannot be in the blacklist, expand it to the actual
    // list of top-level folders as soon as possible.
    if (_oldBlackList == QStringList("/")) {
        _oldBlackList.clear();
        foreach (QString path, list) {
            path.remove(pathToRemove);
            if (path.isEmpty()) {
                continue;
            }
            _oldBlackList.append(path);
        }
    }

    if (!root && list.size() <= 1) {
        //_loading->setText(tr("No subfolders currently on the server."));
        //_loading->resize(_loading->sizeHint()); // because it's not in a layout
        return;
    } else {
        //_loading->hide();
    }

    if (!root) {
        root = new TreeViewItem(_folderTreeWidget);
        root->setText(0, _rootName);
        root->setIcon(0, Theme::instance()->applicationIcon());
        root->setData(0, Qt::UserRole, QString());
        root->setCheckState(0, Qt::Checked);
        qint64 size = job ? job->_sizes.value(pathToRemove, -1) : -1;
        if (size >= 0) {
            root->setText(1, OCC::Utility::octetsToString(size));
            root->setData(1, Qt::UserRole, size); // Display size
            root->setData(1, Qt::UserRole + 1, size); // Full size
        }
    }

    OCC::Utility::sortFilenames(list);
    foreach (QString path, list) {
        auto size = job ? job->_sizes.value(path) : 0;
        path.remove(pathToRemove);
        QStringList paths = path.split('/');
        if (paths.last().isEmpty())
            paths.removeLast();
        if (paths.isEmpty())
            continue;
        if (!path.endsWith('/')) {
            path.append('/');
        }
        recursiveInsert(root, paths, path, size);
    }

    // Root is partially checked if any children are not checked
    for (int i = 0; i < root->childCount(); ++i) {
        const auto child = root->child(i);
        if (child->checkState(0) != Qt::Checked) {
            root->setCheckState(0, Qt::PartiallyChecked);
            break;
        }
    }

    root->setExpanded(true);
}

void ServerFoldersDialog::slotItemExpanded(QTreeWidgetItem *item)
{

}

void ServerFoldersDialog::slotItemChanged(QTreeWidgetItem *item, int col)
{

}

void ServerFoldersDialog::slotLscolFinishedWithError(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::ContentNotFoundError) {
        //_loading->setText(tr("No subfolders currently on the server."));
    } else {
        //_loading->setText(tr("An error occurred while loading the list of sub folders."));
    }
    //_loading->resize(_loading->sizeHint()); // because it's not in a layout
}

}

