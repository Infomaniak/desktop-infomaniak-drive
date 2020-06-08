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

#include "basefoldertreeitemwidget.h"
#include "customtreewidgetitem.h"
#include "guiutility.h"
#include "networkjobs.h"
#include "folderman.h"
#include "accountmanager.h"
#include "common/utility.h"
#include "configfile.h"

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

Q_LOGGING_CATEGORY(lcBaseFolderTreeItemWidget, "foldertreeitemwidget", QtInfoMsg)

BaseFolderTreeItemWidget::BaseFolderTreeItemWidget(const QString &accountId, bool displayRoot, QWidget *parent)
    : QTreeWidget(parent)
    , _accountId(accountId)
    , _displayRoot(displayRoot)
    , _folderIconColor(QColor())
    , _folderIconSize(QSize())
    , _inserting(false)
    , _currentFolderPath(QString())
{
    initUI();

    OCC::ConfigFile::setupDefaultExcludeFilePaths(_excludedFiles);
    _excludedFiles.reloadExcludeFiles();
}

void BaseFolderTreeItemWidget::loadSubFolders()
{
    clear();
    OCC::LsColJob *job = new OCC::LsColJob(getAccountPtr(), getFolderPath(), this);
    job->setProperties(QList<QByteArray>()
                       << "resourcetype"
                       << "http://owncloud.org/ns:size");
    connect(job, &OCC::LsColJob::directoryListingSubfolders, this, &BaseFolderTreeItemWidget::onUpdateDirectories);
    connect(job, &OCC::LsColJob::finishedWithError, this, &BaseFolderTreeItemWidget::onLoadSubFoldersError);
    job->start();
}

void BaseFolderTreeItemWidget::insertPath(QTreeWidgetItem *parent, QStringList pathTrail, QString path, qint64 size)
{
    if (pathTrail.size() == 0) {
        if (path.endsWith('/')) {
            path.chop(1);
        }
        parent->setData(TreeWidgetColumn::Folder, dirRole, path);
    }
    else {
        CustomTreeWidgetItem *item = static_cast<CustomTreeWidgetItem *>(findFirstChild(parent, pathTrail.first()));
        if (!item) {
            item = new CustomTreeWidgetItem(parent);

            // Set icon
            if (pathTrail.first() == commonDocumentsFolderName) {
                setFolderIcon(item, ":/client/resources/icons/document types/folder-common-documents.svg");
            }
            else if (pathTrail.first() == sharedFolderName) {
                setFolderIcon(item, ":/client/resources/icons/document types/folder-disable.svg");
            }
            else {
                setFolderIcon(item, ":/client/resources/icons/actions/folder.svg");
            }

            // Set name
            item->setText(TreeWidgetColumn::Folder, pathTrail.first());

            item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        }

        pathTrail.removeFirst();
        insertPath(item, pathTrail, path, size);
    }
}

void BaseFolderTreeItemWidget::initUI()
{
    setStyleSheet("QTreeWidget::branch:!has-children:adjoins-item { image: none; }"
                  "QTreeWidget::branch:has-children:adjoins-item:open { image: url(:/client/resources/icons/actions/branch-open.svg);"
                  "background-color: transparent; margin-left: 15px; margin-right: 5px; }"
                  "QTreeWidget::branch:has-children:adjoins-item:closed { image: url(:/client/resources/icons/actions/branch-close.svg);"
                  "background-color: transparent; margin-left: 15px; margin-right: 5px; }");

    setSelectionMode(QAbstractItemView::SingleSelection);
    setSortingEnabled(true);
    sortByColumn(TreeWidgetColumn::Folder, Qt::AscendingOrder);
    setColumnCount(2);
    header()->hide();
    header()->setSectionResizeMode(TreeWidgetColumn::Folder, QHeaderView::Stretch);
    header()->setSectionResizeMode(TreeWidgetColumn::Action, QHeaderView::ResizeToContents);
    header()->setStretchLastSection(false);
    setIndentation(treeWidgetIndentation);
    setRootIsDecorated(true);

    connect(this, &QTreeWidget::itemExpanded, this, &BaseFolderTreeItemWidget::onItemExpanded);
    connect(this, &QTreeWidget::currentItemChanged, this, &BaseFolderTreeItemWidget::onCurrentItemChanged);
    connect(this, &QTreeWidget::itemClicked, this, &BaseFolderTreeItemWidget::onItemClicked);
    connect(this, &QTreeWidget::itemDoubleClicked, this, &BaseFolderTreeItemWidget::onItemDoubleClicked);
    connect(this, &QTreeWidget::itemChanged, this, &BaseFolderTreeItemWidget::onItemChanged);
}

void BaseFolderTreeItemWidget::setFolderIcon()
{
    if (_folderIconColor != QColor() && _folderIconSize != QSize()) {
        setIconSize(_folderIconSize);
        if (topLevelItem(0)) {
            setSubFoldersIcon(topLevelItem(0));
        }
    }
}

void BaseFolderTreeItemWidget::setFolderIcon(QTreeWidgetItem *item, const QString &viewIconPath)
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

void BaseFolderTreeItemWidget::setSubFoldersIcon(QTreeWidgetItem *parent)
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

QTreeWidgetItem *BaseFolderTreeItemWidget::findFirstChild(QTreeWidgetItem *parent, const QString &text)
{
    for (int i = 0; i < parent->childCount(); ++i) {
        QTreeWidgetItem *child = parent->child(i);
        if (child->text(TreeWidgetColumn::Folder) == text) {
            return child;
        }
    }
    return 0;
}

OCC::AccountPtr BaseFolderTreeItemWidget::getAccountPtr()
{
    OCC::AccountPtr accountPtr = OCC::AccountManager::instance()->getAccountFromId(_accountId);

    return accountPtr;
}

QString BaseFolderTreeItemWidget::getFolderPath()
{
    return QString();
}

void BaseFolderTreeItemWidget::onUpdateDirectories(QStringList list)
{
    auto job = qobject_cast<OCC::LsColJob *>(sender());
    QScopedValueRollback<bool> isInserting(_inserting);
    _inserting = true;

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

    CustomTreeWidgetItem *root = static_cast<CustomTreeWidgetItem *>(topLevelItem(0));
    if (!root && list.size() <= 1) {
        emit message(tr("No subfolders currently on the server."));
        return;
    } else {
        emit showMessage(false);
    }

    if (!root) {
        root = new CustomTreeWidgetItem(this);

        // Set drive name
        root->setText(TreeWidgetColumn::Folder, getAccountPtr() ? getAccountPtr()->driveName() : QString());
        root->setIcon(TreeWidgetColumn::Folder, OCC::Utility::getIconWithColor(":/client/resources/icons/actions/drive.svg",
                                                                               getAccountPtr()->getDriveColor()));
        root->setData(TreeWidgetColumn::Folder, dirRole, QString());
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

    root->setExpanded(true);
    if (!_displayRoot) {
        setRootIndex(indexFromItem(root));
    }
}

void BaseFolderTreeItemWidget::onLoadSubFoldersError(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::ContentNotFoundError) {
        emit message(tr("No subfolders currently on the server."));
    } else {
        emit message(tr("An error occurred while loading the list of sub folders."));
    }
}

void BaseFolderTreeItemWidget::onItemExpanded(QTreeWidgetItem *item)
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
    connect(job, &OCC::LsColJob::directoryListingSubfolders, this, &BaseFolderTreeItemWidget::onUpdateDirectories);
    job->start();
}

void BaseFolderTreeItemWidget::onCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    if (previous) {
        // Remove action icon
        previous->setIcon(TreeWidgetColumn::Action, QIcon());
    }

    if (current && !current->text(TreeWidgetColumn::Folder).isEmpty()) {
        // Add action icon
        current->setIcon(TreeWidgetColumn::Action, OCC::Utility::getIconWithColor(":/client/resources/icons/actions/folder-add.svg"));

        _currentFolderPath = current->data(TreeWidgetColumn::Folder, dirRole).toString();
        emit folderSelected(_currentFolderPath);
    }
}

void BaseFolderTreeItemWidget::onItemClicked(QTreeWidgetItem *item, int column)
{
    if (column == TreeWidgetColumn::Action && !item->text(TreeWidgetColumn::Folder).isEmpty()) {
        // Add Folder
        CustomTreeWidgetItem *newItem = new CustomTreeWidgetItem(item);

        // Set icon
        setFolderIcon(newItem, ":/client/resources/icons/actions/folder.svg");

        // Set name
        newItem->setText(TreeWidgetColumn::Folder, QString());

        // Expand item
        item->setExpanded(true);

        // Select new item
        setCurrentItem(newItem);

        // Allow editing new item name
        newItem->setFlags(newItem->flags() | Qt::ItemIsEditable);
        editItem(newItem, TreeWidgetColumn::Folder);
    }
}

void BaseFolderTreeItemWidget::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (column == TreeWidgetColumn::Folder && item->flags() & Qt::ItemIsEditable) {
        // Allow editing item name
        editItem(item, TreeWidgetColumn::Folder);
    }
}

void BaseFolderTreeItemWidget::onItemChanged(QTreeWidgetItem *item, int column)
{
    if (column == TreeWidgetColumn::Folder && item->flags() & Qt::ItemIsEditable) {
        // Set path
        QString path = item->parent()->data(TreeWidgetColumn::Folder, dirRole).toString()
                + "/" + item->text(TreeWidgetColumn::Folder);
        item->setData(TreeWidgetColumn::Folder, dirRole, path);
        onCurrentItemChanged(item, nullptr);
        scrollToItem(item);
    }
}

}
