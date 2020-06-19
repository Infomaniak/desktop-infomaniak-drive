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

#include "foldertreeitemwidget.h"
#include "guiutility.h"
#include "networkjobs.h"
#include "accountmanager.h"
#include "folderman.h"
#include "accountmanager.h"
#include "common/utility.h"
#include "configfile.h"

#include <QDir>
#include <QHeaderView>
#include <QMutableListIterator>
#include <QScopedValueRollback>

namespace KDC {

static const int treeWidgetIndentation = 30;

static const QString commonDocumentsFolderName("Common documents");
static const QString sharedFolderName("Shared");

// 1st column roles
static const int viewIconPathRole = Qt::UserRole;
static const int dirRole = Qt::UserRole + 1;

// 2nd column roles
static const int sizeRole = Qt::UserRole;

Q_LOGGING_CATEGORY(lcFolderTreeItemWidget, "foldertreeitemwidget", QtInfoMsg)

FolderTreeItemWidget::FolderTreeItemWidget(const QString &folderId, bool displayRoot, QWidget *parent)
    : QTreeWidget(parent)
    , _folderId(folderId)
    , _accountId(QString())
    , _folderPath(QString())
    , _oldBlackList(QStringList())
    , _displayRoot(displayRoot)
    , _mode(Update)
    , _currentFolder(nullptr)
    , _folderIconColor(QColor())
    , _folderIconSize(QSize())
    , _sizeTextColor(QColor())
    , _inserting(false)
{
    initUI();

    _currentFolder = OCC::FolderMan::instance()->folder(_folderId);
    if (!_currentFolder) {
        qCDebug(lcFolderTreeItemWidget) << "Folder not found: " << _folderId;
        return;
    }

    // Make sure we don't get crashes if the folder is destroyed while the dialog is still opened
    connect(_currentFolder, &QObject::destroyed, this, &QObject::deleteLater);
}

FolderTreeItemWidget::FolderTreeItemWidget(const QString &accountId, const QString &serverFolderPath, bool displayRoot, QWidget *parent)
    : QTreeWidget(parent)
    , _folderId(QString())
    , _accountId(accountId)
    , _folderPath(serverFolderPath)
    , _oldBlackList(QStringList())
    , _displayRoot(displayRoot)
    , _mode(Creation)
    , _currentFolder(nullptr)
    , _folderIconColor(QColor())
    , _folderIconSize(QSize())
    , _sizeTextColor(QColor())
    , _inserting(false)
{
    initUI();
}

void FolderTreeItemWidget::loadSubFolders()
{
    if (_currentFolder) {
        bool ok;
        _oldBlackList = _currentFolder->journalDb()->getSelectiveSyncList(OCC::SyncJournalDb::SelectiveSyncBlackList, &ok);
    }

    OCC::ConfigFile::setupDefaultExcludeFilePaths(_excludedFiles);
    _excludedFiles.reloadExcludeFiles();

    clear();
    OCC::LsColJob *job = new OCC::LsColJob(getAccountPtr(), getFolderPath(), this);
    job->setProperties(QList<QByteArray>()
                       << "resourcetype"
                       << "http://owncloud.org/ns:size");
    connect(job, &OCC::LsColJob::directoryListingSubfolders, this, &FolderTreeItemWidget::onUpdateDirectories);
    connect(job, &OCC::LsColJob::finishedWithError, this, &FolderTreeItemWidget::onLoadSubFoldersError);
    job->start();
}

void FolderTreeItemWidget::insertPath(QTreeWidgetItem *parent, QStringList pathTrail, QString path, qint64 size)
{
    if (pathTrail.size() == 0) {
        if (path.endsWith(dirSeparator)) {
            path.chop(1);
        }
        parent->setData(TreeWidgetColumn::Folder, dirRole, path);
    } else {
        QString folderName = pathTrail.first();
        CustomTreeWidgetItem *item = static_cast<CustomTreeWidgetItem *>(findFirstChild(parent, folderName));
        if (!item) {
            item = new CustomTreeWidgetItem(parent);

            // Set check status
            if (parent->checkState(TreeWidgetColumn::Folder) == Qt::Checked
                || parent->checkState(TreeWidgetColumn::Folder) == Qt::PartiallyChecked) {
                item->setCheckState(TreeWidgetColumn::Folder, Qt::Checked);
                foreach (const QString &str, _oldBlackList) {
                    if (str == path || str == dirSeparator) {
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
            setFolderIcon(item, folderName);

            // Set name
            item->setText(TreeWidgetColumn::Folder, folderName);

            // Set size
            if (size >= 0) {
                item->setText(TreeWidgetColumn::Size, OCC::Utility::octetsToString(size));
                item->setTextColor(TreeWidgetColumn::Size, _sizeTextColor);
                item->setTextAlignment(TreeWidgetColumn::Size, Qt::AlignRight | Qt::AlignVCenter);
                item->setData(TreeWidgetColumn::Size, sizeRole, size);
            }

            item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        }

        pathTrail.removeFirst();
        insertPath(item, pathTrail, path, size);
    }
}

QStringList FolderTreeItemWidget::createBlackList(QTreeWidgetItem *root) const
{
    if (!root) {
        root = topLevelItem(0);
    }
    if (!root) {
        return QStringList();
    }

    switch (root->checkState(TreeWidgetColumn::Folder)) {
    case Qt::Unchecked:
        return QStringList(root->data(TreeWidgetColumn::Folder, dirRole).toString() + dirSeparator);
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

qint64 FolderTreeItemWidget::nodeSize(QTreeWidgetItem *item) const
{
    qint64 size = 0;
    if (item->checkState(TreeWidgetColumn::Folder) == Qt::Checked) {
        size = item->data(TreeWidgetColumn::Size, sizeRole).toLongLong();
    }
    else if (item->checkState(TreeWidgetColumn::Folder) == Qt::PartiallyChecked) {
        size = item->data(TreeWidgetColumn::Size, sizeRole).toLongLong();

        // Remove the size of unchecked subfolders
        for (int i = 0; i < item->childCount(); ++i) {
            if (item->child(i)->checkState(TreeWidgetColumn::Folder) == Qt::PartiallyChecked) {
                size -= nodeSize(item->child(i));
            }
            else if (item->child(i)->checkState(TreeWidgetColumn::Folder) == Qt::Unchecked) {
                size -= item->child(i)->data(TreeWidgetColumn::Size, sizeRole).toLongLong();
            }
        }
    }
    return size;
}

void FolderTreeItemWidget::initUI()
{
    setSelectionMode(QAbstractItemView::NoSelection);
    setSortingEnabled(true);
    sortByColumn(TreeWidgetColumn::Folder, Qt::AscendingOrder);
    setColumnCount(2);
    header()->hide();
    header()->setSectionResizeMode(TreeWidgetColumn::Folder, QHeaderView::Stretch);
    header()->setSectionResizeMode(TreeWidgetColumn::Size, QHeaderView::ResizeToContents);
    header()->setStretchLastSection(false);
    setIndentation(treeWidgetIndentation);
    setRootIsDecorated(!_displayRoot);

    connect(this, &QTreeWidget::itemExpanded, this, &FolderTreeItemWidget::onItemExpanded);
    connect(this, &QTreeWidget::itemChanged, this, &FolderTreeItemWidget::onItemChanged);
}

QString FolderTreeItemWidget::iconPath(const QString &folderName)
{
    QString iconPath;
    if (folderName == commonDocumentsFolderName) {
        iconPath = ":/client/resources/icons/document types/folder-common-documents.svg";
    }
    else if (folderName == sharedFolderName) {
        iconPath = ":/client/resources/icons/document types/folder-disable.svg";
    }
    else {
        iconPath = ":/client/resources/icons/actions/folder.svg";
    }
    return iconPath;
}

QColor FolderTreeItemWidget::iconColor(const QString &folderName)
{
    QColor iconColor;
    if (folderName == commonDocumentsFolderName) {
        iconColor = QColor();
    }
    else if (folderName == sharedFolderName) {
        iconColor = QColor();
    }
    else {
        iconColor = _folderIconColor;
    }
    return iconColor;
}

void FolderTreeItemWidget::setFolderIcon()
{
    if (_folderIconColor != QColor() && _folderIconSize != QSize()) {
        setIconSize(_folderIconSize);
        if (topLevelItem(0)) {
            setSubFoldersIcon(topLevelItem(0));
        }
    }
}

void FolderTreeItemWidget::setFolderIcon(QTreeWidgetItem *item, const QString &folderName)
{
    if (item) {
        if (item->data(TreeWidgetColumn::Folder, viewIconPathRole).isNull()) {
            item->setData(TreeWidgetColumn::Folder, viewIconPathRole, iconPath(folderName));
        }
        if (_folderIconColor != QColor() && _folderIconSize != QSize()) {
            item->setIcon(TreeWidgetColumn::Folder,
                          OCC::Utility::getIconWithColor(iconPath(folderName), iconColor(folderName)));
        }
    }
}

void FolderTreeItemWidget::setSubFoldersIcon(QTreeWidgetItem *parent)
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

QTreeWidgetItem *FolderTreeItemWidget::findFirstChild(QTreeWidgetItem *parent, const QString &text)
{
    for (int i = 0; i < parent->childCount(); ++i) {
        QTreeWidgetItem *child = parent->child(i);
        if (child->text(TreeWidgetColumn::Folder) == text) {
            return child;
        }
    }
    return 0;
}

OCC::AccountPtr FolderTreeItemWidget::getAccountPtr()
{
    if (_accountPtr.isNull()) {
        if (_mode == Update) {
            if (_currentFolder && _currentFolder->accountState()) {
                _accountPtr = _currentFolder->accountState()->account();
            }
            else {
                qCDebug(lcFolderTreeItemWidget) << "Null pointer";
                return nullptr;
            }
        }
        else {
            if (!_accountId.isEmpty()) {
                _accountPtr = OCC::AccountManager::instance()->getAccountFromId(_accountId);
            }
            else {
                qCDebug(lcFolderTreeItemWidget) << "No account id";
                return nullptr;
            }
        }
    }

    return _accountPtr;
}

QString FolderTreeItemWidget::getFolderPath()
{
    QString folderPath;
    if (_mode == Update) {
        if (_currentFolder) {
            folderPath = _currentFolder->remotePath();
        }
        else {
            qCDebug(lcFolderTreeItemWidget) << "Null pointer";
            return QString();
        }
    }
    else {
        folderPath = _folderPath;
    }

    return folderPath.startsWith(dirSeparator) ? folderPath.mid(1) : folderPath;
}

void FolderTreeItemWidget::onUpdateDirectories(QStringList list)
{
    auto job = qobject_cast<OCC::LsColJob *>(sender());
    QScopedValueRollback<bool> isInserting(_inserting);
    _inserting = true;

    QUrl url = getAccountPtr() ? getAccountPtr()->davUrl() : QUrl();
    QString pathToRemove = url.path();
    if (!pathToRemove.endsWith(dirSeparator)) {
        pathToRemove.append(dirSeparator);
    }
    pathToRemove.append(getFolderPath());
    if (!pathToRemove.endsWith(dirSeparator)) {
        pathToRemove.append(dirSeparator);
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
    if (_oldBlackList == QStringList(dirSeparator)) {
        _oldBlackList.clear();
        for (QString path : list) {
            path.remove(pathToRemove);
            if (path.isEmpty()) {
                continue;
            }
            _oldBlackList.append(path);
        }
    }

    CustomTreeWidgetItem *root = static_cast<CustomTreeWidgetItem *>(topLevelItem(0));
    if (!root && list.size() <= 1) {
        // No sub folders
        emit terminated(false, true);
        return;
    } else {
        emit terminated(false, false);
    }

    if (!root) {
        root = new CustomTreeWidgetItem(this);
        if (_folderPath.isEmpty()) {
            // Set drive name
            root->setText(TreeWidgetColumn::Folder, getAccountPtr() ? getAccountPtr()->driveName() : QString());
            root->setIcon(TreeWidgetColumn::Folder, OCC::Utility::getIconWithColor(":/client/resources/icons/actions/drive.svg",
                                                                                   getAccountPtr()->getDriveColor()));
            root->setData(TreeWidgetColumn::Folder, dirRole, QString());
        }
        else {
            // Set folder name
            QDir dir(_folderPath);
            root->setText(TreeWidgetColumn::Folder, dir.dirName());
            setFolderIcon(root, ":/client/resources/icons/actions/folder.svg");
            root->setData(TreeWidgetColumn::Folder, dirRole, _folderPath);
        }

        // Set check state
        root->setCheckState(TreeWidgetColumn::Folder, Qt::Checked);

        // Set size
        qint64 size = job ? job->_sizes.value(pathToRemove, -1) : -1;
        if (size >= 0) {
            root->setText(TreeWidgetColumn::Size, OCC::Utility::octetsToString(size));
            root->setTextAlignment(TreeWidgetColumn::Size, Qt::AlignRight | Qt::AlignVCenter);
            root->setData(TreeWidgetColumn::Size, sizeRole, size);
        }
    }

    OCC::Utility::sortFilenames(list);
    foreach (QString path, list) {
        auto size = job ? job->_sizes.value(path) : 0;
        path.remove(pathToRemove);
        QStringList paths = path.split(dirSeparator);
        if (paths.last().isEmpty()) {
            paths.removeLast();
        }
        if (paths.isEmpty()) {
            continue;
        }
        if (!path.endsWith(dirSeparator)) {
            path.append(dirSeparator);
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
    if (!_displayRoot) {
        setRootIndex(indexFromItem(root));
    }
}

void FolderTreeItemWidget::onLoadSubFoldersError(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::ContentNotFoundError) {
        // No sub folders
        emit terminated(false, true);
    } else {
        emit terminated(true);
    }
}

void FolderTreeItemWidget::onItemExpanded(QTreeWidgetItem *item)
{
    item->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicatorWhenChildless);

    QString dir = item->data(TreeWidgetColumn::Folder, dirRole).toString();
    if (dir.isEmpty()) {
        return;
    }

    QString folderPath = getFolderPath();
    if (!folderPath.isEmpty()) {
        folderPath.append(dirSeparator);
    }
    folderPath.append(dir);

    OCC::LsColJob *job = new OCC::LsColJob(getAccountPtr(), folderPath, this);
    job->setProperties(QList<QByteArray>() << "resourcetype"
                                           << "http://owncloud.org/ns:size");
    connect(job, &OCC::LsColJob::directoryListingSubfolders, this, &FolderTreeItemWidget::onUpdateDirectories);
    job->start();
}

void FolderTreeItemWidget::onItemChanged(QTreeWidgetItem *item, int col)
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
                onItemChanged(parent, col);
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
                onItemChanged(parent, col);
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
                onItemChanged(parent, col);
            }
        }
    }

    emit needToSave();
}

}
