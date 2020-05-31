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
#include "accountmanager.h"
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

static const int boxHMargin = 40;
static const int boxHSpacing = 10;
static const int titleBoxVMargin = 14;
static const int descriptionBoxVMargin = 15;
static const int availableSpaceBoxVMargin = 20;
static const int messageVMargin = 20;
static const int folderTreeBoxVMargin = 20;
static const int treeWidgetIndentation = 30;

// 1st column roles
static const int viewIconPathRole = Qt::UserRole;
static const int dirRole = Qt::UserRole + 1;

// 2nd column roles
static const int sizeRole = Qt::UserRole;

Q_LOGGING_CATEGORY(lcServerFoldersDialog, "serverfoldersdialog", QtInfoMsg)

ServerFoldersDialog::ServerFoldersDialog(const AccountInfo *accountInfo, QWidget *parent)
    : CustomDialog(true, parent)
    , _accountInfo(accountInfo)
    , _currentFolder(nullptr)
    , _oldBlackList(QStringList())
    , _inserting(false)
    , _infoIconLabel(nullptr)
    , _availableSpaceTextLabel(nullptr)
    , _messageLabel(nullptr)
    , _folderTreeWidget(nullptr)
    , _saveButton(nullptr)
    , _infoIconColor(QColor())
    , _infoIconSize(QSize())
    , _needToSave(false)
{
    setStyleSheet("QTreeWidget::indicator:checked { image: url(:/client/resources/icons/actions/checkbox-checked.svg); }"
                  "QTreeWidget::indicator:unchecked { image: url(:/client/resources/icons/actions/checkbox-unchecked.svg); }"
                  "QTreeWidget::indicator:indeterminate { image: url(:/client/resources/icons/actions/checkbox-indeterminate.svg); }"
                  "QTreeWidget::indicator:checked:disabled { image: url(:/client/resources/icons/actions/checkbox-checked.svg); }"
                  "QTreeWidget::indicator:unchecked:disabled { image: url(:/client/resources/icons/actions/checkbox-unchecked.svg); }"
                  "QTreeWidget::indicator:indeterminate:disabled { image: url(:/client/resources/icons/actions/checkbox-indeterminate.svg); }"
                  "QTreeWidget::branch:!has-children:adjoins-item { image: none; }"
                  "QTreeWidget::branch:has-children:adjoins-item:open { image: url(:/client/resources/icons/actions/branch-open.svg); margin-left: 20px; margin-right: 0px; }"
                  "QTreeWidget::branch:has-children:adjoins-item:closed { image: url(:/client/resources/icons/actions/branch-close.svg); margin-left: 20px; margin-right: 0px; }");

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
    _availableSpaceTextLabel->setObjectName("largeMediumTextLabel");
    availableSpaceHBox->addWidget(_availableSpaceTextLabel);
    availableSpaceHBox->addStretch();

    // Message
    _messageLabel = new QLabel(this);
    _messageLabel->setObjectName("messageLabel");
    _messageLabel->setText(tr("Loading..."));
    _messageLabel->setVisible(false);
    _messageLabel->setContentsMargins(boxHMargin, 0, boxHMargin, messageVMargin);
    mainLayout->addWidget(_messageLabel);

    // Folder tree
    QHBoxLayout *folderTreeHBox = new QHBoxLayout();
    folderTreeHBox->setContentsMargins(boxHMargin, 0, boxHMargin, folderTreeBoxVMargin);
    mainLayout->addLayout(folderTreeHBox);

    _folderTreeWidget = new QTreeWidget(this);
    _folderTreeWidget->setSelectionMode(QAbstractItemView::NoSelection);
    _folderTreeWidget->setSortingEnabled(true);
    _folderTreeWidget->sortByColumn(TreeWidgetColumn::Folder, Qt::AscendingOrder);
    _folderTreeWidget->setColumnCount(2);
    _folderTreeWidget->header()->hide();
    _folderTreeWidget->header()->setSectionResizeMode(TreeWidgetColumn::Folder, QHeaderView::Stretch);
    _folderTreeWidget->header()->setSectionResizeMode(TreeWidgetColumn::Size, QHeaderView::ResizeToContents);
    _folderTreeWidget->header()->setStretchLastSection(false);
    _folderTreeWidget->setRootIsDecorated(false);
    _folderTreeWidget->setIndentation(treeWidgetIndentation);
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

    connect(_folderTreeWidget, &QTreeWidget::itemExpanded, this, &ServerFoldersDialog::slotItemExpanded);
    connect(_folderTreeWidget, &QTreeWidget::itemChanged, this, &ServerFoldersDialog::slotItemChanged);
    connect(_saveButton, &QPushButton::clicked, this, &ServerFoldersDialog::onSaveButtonTriggered);
    connect(cancelButton, &QPushButton::clicked, this, &ServerFoldersDialog::onExit);
    connect(this, &CustomDialog::exit, this, &ServerFoldersDialog::onExit);
}

void ServerFoldersDialog::updateUI()
{
    if (_accountInfo->_folderMap.size() == 0) {
        return;
    }

    QString folderId = _accountInfo->_folderMap.begin()->first;
    _currentFolder = OCC::FolderMan::instance()->folder(folderId);
    if (!_currentFolder) {
        qCDebug(lcServerFoldersDialog) << "Folder not found: " << folderId;
        return;
    }

    // Make sure we don't get crashes if the folder is destroyed while the dialog is still opened
    connect(_currentFolder, &QObject::destroyed, this, &QObject::deleteLater);

    // Available space
    qint64 freeBytes = OCC::Utility::freeDiskSpace(_currentFolder->remotePath());
    _availableSpaceTextLabel->setText(tr("Space available on your computer for the current folder : %1")
                                      .arg(OCC::Utility::octetsToString(freeBytes)));

    bool ok;
    _oldBlackList = _currentFolder->journalDb()->getSelectiveSyncList(OCC::SyncJournalDb::SelectiveSyncBlackList, &ok);

    // Get subfolders
    OCC::LsColJob *job = new OCC::LsColJob(getAccountPtr(), getFolderPath(), this);
    job->setProperties(QList<QByteArray>()
                       << "resourcetype"
                       << "http://owncloud.org/ns:size");
    connect(job, &OCC::LsColJob::directoryListingSubfolders, this, &ServerFoldersDialog::slotUpdateDirectories);
    connect(job, &OCC::LsColJob::finishedWithError, this, &ServerFoldersDialog::slotLscolFinishedWithError);
    job->start();

    _folderTreeWidget->clear();
    _messageLabel->show();
}

void ServerFoldersDialog::setFolderIcon()
{
    if (_folderIconColor != QColor() && _folderIconSize != QSize()) {
        _folderTreeWidget->setIconSize(_folderIconSize);
        if (_folderTreeWidget->topLevelItem(0)) {
            setSubFoldersIcon(_folderTreeWidget->topLevelItem(0));
        }
    }
}

void ServerFoldersDialog::setFolderIcon(QTreeWidgetItem *item, const QString &viewIconPath)
{
    if (item) {
        if (item->data(TreeWidgetColumn::Folder, viewIconPathRole).isNull()) {
            item->setData(TreeWidgetColumn::Folder, viewIconPathRole, viewIconPath);
        }
        if (_folderIconColor != QColor() && _folderIconSize != QSize()) {
            item->setIcon(TreeWidgetColumn::Folder,
                          OCC::Utility::getIconWithColor(viewIconPath, _folderIconColor));
        }
    }
}

void ServerFoldersDialog::setSubFoldersIcon(QTreeWidgetItem *parent)
{
    for (int i = 0; i < parent->childCount(); ++i) {
        QTreeWidgetItem *item = parent->child(i);
        QVariant viewIconPathV = item->data(TreeWidgetColumn::Folder, viewIconPathRole);
        if (!viewIconPathV.isNull()) {
            QString viewIconPath = qvariant_cast<QString>(viewIconPathV);
            setFolderIcon(item, viewIconPath);
        }
        setSubFoldersIcon(item);
    }
}

void ServerFoldersDialog::setNeedToSave(bool value)
{
    _needToSave = value;
    _saveButton->setEnabled(value);
}

QTreeWidgetItem *ServerFoldersDialog::findFirstChild(QTreeWidgetItem *parent, const QString &text)
{
    for (int i = 0; i < parent->childCount(); ++i) {
        QTreeWidgetItem *child = parent->child(i);
        if (child->text(TreeWidgetColumn::Folder) == text) {
            return child;
        }
    }
    return 0;
}

void ServerFoldersDialog::insertPath(QTreeWidgetItem *parent, QStringList pathTrail, QString path, qint64 size)
{
    if (pathTrail.size() == 0) {
        if (path.endsWith('/')) {
            path.chop(1);
        }
        parent->setData(TreeWidgetColumn::Folder, dirRole, path);
    } else {
        TreeViewItem *item = static_cast<TreeViewItem *>(findFirstChild(parent, pathTrail.first()));
        if (!item) {
            item = new TreeViewItem(parent);

            // Set check status
            if (parent->checkState(TreeWidgetColumn::Folder) == Qt::Checked
                || parent->checkState(TreeWidgetColumn::Folder) == Qt::PartiallyChecked) {
                item->setCheckState(TreeWidgetColumn::Folder, Qt::Checked);
                foreach (const QString &str, _oldBlackList) {
                    if (str == path || str == QLatin1String("/")) {
                        item->setCheckState(TreeWidgetColumn::Folder, Qt::Unchecked);
                        break;
                    } else if (str.startsWith(path)) {
                        item->setCheckState(TreeWidgetColumn::Folder, Qt::PartiallyChecked);
                    }
                }
            } else if (parent->checkState(TreeWidgetColumn::Folder) == Qt::Unchecked) {
                item->setCheckState(TreeWidgetColumn::Folder, Qt::Unchecked);
            }

            // Set icon
            setFolderIcon(item, ":/client/resources/icons/actions/folder.svg");

            // Set name
            item->setText(TreeWidgetColumn::Folder, pathTrail.first());

            // Set size
            if (size >= 0) {
                item->setText(TreeWidgetColumn::Size, OCC::Utility::octetsToString(size));
                item->setTextColor(TreeWidgetColumn::Size, _sizeTextColor);
                item->setTextAlignment(TreeWidgetColumn::Size, Qt::AlignRight);
                item->setData(TreeWidgetColumn::Size, sizeRole, size);
            }

            item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        }

        pathTrail.removeFirst();
        insertPath(item, pathTrail, path, size);
    }
}

qint64 ServerFoldersDialog::selectionSize(QTreeWidgetItem *root)
{
    if (!root) {
        root = _folderTreeWidget->topLevelItem(0);
    }
    if (!root) {
        return -1;
    }

    switch (root->checkState(TreeWidgetColumn::Folder)) {
    case Qt::Unchecked:
        return 0;
    case Qt::Checked:
        return root->data(TreeWidgetColumn::Size, sizeRole).toLongLong();
    case Qt::PartiallyChecked:
        break;
    }

    qint64 size = 0;
    if (root->childCount()) {
        size = root->data(TreeWidgetColumn::Size, sizeRole).toLongLong();
        for (int i = 0; i < root->childCount(); ++i) {
            if (root->child(i)->checkState(TreeWidgetColumn::Folder) == Qt::Unchecked) {
                size -= root->child(i)->data(TreeWidgetColumn::Size, sizeRole).toLongLong();
            }
            else if (root->child(i)->checkState(TreeWidgetColumn::Folder) == Qt::PartiallyChecked) {
                auto childSize = selectionSize(root->child(i));
                if (childSize < 0) {
                    return childSize;
                }

                size -= root->child(i)->data(TreeWidgetColumn::Size, sizeRole).toLongLong() - childSize;
            }
        }
    } else {
        // We did not load from the server so we have no idea how much we will sync from this branch
        return -1;
    }
    return size;
}

QStringList ServerFoldersDialog::createBlackList(QTreeWidgetItem *root) const
{
    if (!root) {
        root = _folderTreeWidget->topLevelItem(0);
    }
    if (!root) {
        return QStringList();
    }

    switch (root->checkState(TreeWidgetColumn::Folder)) {
    case Qt::Unchecked:
        return QStringList(root->data(TreeWidgetColumn::Folder, dirRole).toString() + "/");
    case Qt::Checked:
        return QStringList();
    case Qt::PartiallyChecked:
        break;
    }

    QStringList result;
    if (root->childCount()) {
        for (int i = 0; i < root->childCount(); ++i) {
            result += createBlackList(root->child(i));
        }
    } else {
        // We did not load from the server so we re-use the one from the old black list
        QString path = root->data(TreeWidgetColumn::Folder, dirRole).toString();
        foreach (const QString &it, _oldBlackList) {
            if (it.startsWith(path)) {
                result += it;
            }
        }
    }
    return result;
}

OCC::AccountPtr ServerFoldersDialog::getAccountPtr()
{
    if (_currentFolder && _currentFolder->accountState()) {
        return _currentFolder->accountState()->account();
    }
    else {
        qCDebug(lcServerFoldersDialog) << "Null pointer";
        return nullptr;
    }
}

QString ServerFoldersDialog::getFolderPath()
{
    if (_currentFolder) {
        QString folderPath = _currentFolder->remotePath().startsWith(QLatin1Char('/'))
                ? _currentFolder->remotePath().mid(1)
                : _currentFolder->remotePath();
        return folderPath;
    }
    else {
        qCDebug(lcServerFoldersDialog) << "Null pointer";
        return QString();
    }
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
    Q_UNUSED(checked)

    bool ok;
    auto oldBlackListSet = _currentFolder->journalDb()->getSelectiveSyncList(OCC::SyncJournalDb::SelectiveSyncBlackList, &ok).toSet();
    if (!ok) {
        return;
    }

    QStringList blackList = createBlackList();
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

void ServerFoldersDialog::slotUpdateDirectories(QStringList list)
{
    auto job = qobject_cast<OCC::LsColJob *>(sender());
    QScopedValueRollback<bool> isInserting(_inserting);
    _inserting = true;

    TreeViewItem *root = static_cast<TreeViewItem *>(_folderTreeWidget->topLevelItem(0));

    QUrl url = getAccountPtr() ? getAccountPtr()->davUrl() : QUrl();
    QString pathToRemove = url.path();
    if (!pathToRemove.endsWith('/')) {
        pathToRemove.append('/');
    }
    pathToRemove.append(getFolderPath());
    if (!pathToRemove.endsWith('/')) {
        pathToRemove.append('/');
    }

    // Check for excludes.
    QMutableListIterator<QString> it(list);
    while (it.hasNext()) {
        it.next();
        if (_excludedFiles.isExcluded(it.value(), pathToRemove, OCC::FolderMan::instance()->ignoreHiddenFiles())) {
            it.remove();
        }
    }

    // Since / cannot be in the blacklist, expand it to the actual list of top-level folders as soon as possible.
    if (_oldBlackList == QStringList("/")) {
        _oldBlackList.clear();
        for (QString path : list) {
            path.remove(pathToRemove);
            if (path.isEmpty()) {
                continue;
            }
            _oldBlackList.append(path);
        }
    }

    if (!root && list.size() <= 1) {
        _messageLabel->setText(tr("No subfolders currently on the server."));
        return;
    } else {
        _messageLabel->hide();
    }

    if (!root) {
        root = new TreeViewItem(_folderTreeWidget);
        QFont font = _folderTreeWidget->font();
        font.setWeight(OCC::Utility::getQFontWeightFromQSSFontWeight(_headerFontWeight));
        root->setFont(TreeWidgetColumn::Folder, font);

        // Set drive name
        root->setText(TreeWidgetColumn::Folder, getAccountPtr() ? getAccountPtr()->driveName() : QString());
        root->setIcon(TreeWidgetColumn::Folder, OCC::Utility::getIconWithColor(":/client/resources/icons/actions/drive.svg",
                                                                               _accountInfo->_color));
        root->setData(TreeWidgetColumn::Folder, dirRole, QString());

        // Set check state
        root->setCheckState(TreeWidgetColumn::Folder, Qt::Checked);

        // Set size
        qint64 size = job ? job->_sizes.value(pathToRemove, -1) : -1;
        if (size >= 0) {
            root->setFont(TreeWidgetColumn::Size, font);
            root->setText(TreeWidgetColumn::Size, OCC::Utility::octetsToString(size));
            root->setTextAlignment(TreeWidgetColumn::Size, Qt::AlignRight);
            root->setData(TreeWidgetColumn::Size, sizeRole, size);
        }
    }

    OCC::Utility::sortFilenames(list);
    foreach (QString path, list) {
        auto size = job ? job->_sizes.value(path) : 0;
        path.remove(pathToRemove);
        QStringList paths = path.split('/');
        if (paths.last().isEmpty()) {
            paths.removeLast();
        }
        if (paths.isEmpty()) {
            continue;
        }
        if (!path.endsWith('/')) {
            path.append('/');
        }
        insertPath(root, paths, path, size);
    }

    // Root is partially checked if any children are not checked
    for (int i = 0; i < root->childCount(); ++i) {
        const auto child = root->child(i);
        if (child->checkState(TreeWidgetColumn::Folder) != Qt::Checked) {
            root->setCheckState(TreeWidgetColumn::Folder, Qt::PartiallyChecked);
            break;
        }
    }

    root->setExpanded(true);
}

void ServerFoldersDialog::slotItemExpanded(QTreeWidgetItem *item)
{
    item->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicatorWhenChildless);

    QString dir = item->data(TreeWidgetColumn::Folder, dirRole).toString();
    if (dir.isEmpty()) {
        return;
    }

    QString folderPath = getFolderPath();
    if (!folderPath.isEmpty()) {
        folderPath.append('/');
    }
    folderPath.append(dir);

    OCC::LsColJob *job = new OCC::LsColJob(getAccountPtr(), folderPath, this);
    job->setProperties(QList<QByteArray>() << "resourcetype"
                                           << "http://owncloud.org/ns:size");
    connect(job, &OCC::LsColJob::directoryListingSubfolders, this, &ServerFoldersDialog::slotUpdateDirectories);
    job->start();
}

void ServerFoldersDialog::slotItemChanged(QTreeWidgetItem *item, int col)
{
    if (col != TreeWidgetColumn::Folder || _inserting) {
        return;
    }

    if (item->checkState(TreeWidgetColumn::Folder) == Qt::Checked) {
        // Need to check the parent as well if all the siblings are also checked
        QTreeWidgetItem *parent = item->parent();
        if (parent && parent->checkState(TreeWidgetColumn::Folder) != Qt::Checked) {
            bool hasUnchecked = false;
            for (int i = 0; i < parent->childCount(); ++i) {
                if (parent->child(i)->checkState(TreeWidgetColumn::Folder) != Qt::Checked) {
                    hasUnchecked = true;
                    break;
                }
            }
            if (!hasUnchecked) {
                parent->setCheckState(TreeWidgetColumn::Folder, Qt::Checked);
            } else if (parent->checkState(TreeWidgetColumn::Folder) == Qt::Unchecked) {
                parent->setCheckState(TreeWidgetColumn::Folder, Qt::PartiallyChecked);
            } else {
                // Refresh parent
                slotItemChanged(parent, col);
            }
        }

        // Check all the children
        for (int i = 0; i < item->childCount(); ++i) {
            if (item->child(i)->checkState(TreeWidgetColumn::Folder) != Qt::Checked) {
                item->child(i)->setCheckState(TreeWidgetColumn::Folder, Qt::Checked);
            }
        }
    }

    if (item->checkState(TreeWidgetColumn::Folder) == Qt::Unchecked) {
        QTreeWidgetItem *parent = item->parent();
        if (parent) {
            if (parent->checkState(TreeWidgetColumn::Folder) == Qt::Checked) {
                parent->setCheckState(TreeWidgetColumn::Folder, Qt::PartiallyChecked);
            }
            else {
                // Refresh parent
                slotItemChanged(parent, col);
            }
        }

        // Uncheck all the children
        for (int i = 0; i < item->childCount(); ++i) {
            if (item->child(i)->checkState(TreeWidgetColumn::Folder) != Qt::Unchecked) {
                item->child(i)->setCheckState(TreeWidgetColumn::Folder, Qt::Unchecked);
            }
        }

        // Can't uncheck the root.
        if (!parent) {
            item->setCheckState(TreeWidgetColumn::Folder, Qt::PartiallyChecked);
        }
    }

    if (item->checkState(TreeWidgetColumn::Folder) == Qt::PartiallyChecked) {
        QTreeWidgetItem *parent = item->parent();
        if (parent) {
            if (parent->checkState(TreeWidgetColumn::Folder) == Qt::Checked) {
                parent->setCheckState(TreeWidgetColumn::Folder, Qt::PartiallyChecked);
            }
            else {
                // Refresh parent
                slotItemChanged(parent, col);
            }
        }
    }

    setNeedToSave(true);
}

void ServerFoldersDialog::slotLscolFinishedWithError(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::ContentNotFoundError) {
        _messageLabel->setText(tr("No subfolders currently on the server."));
    } else {
        _messageLabel->setText(tr("An error occurred while loading the list of sub folders."));
    }
}

}

