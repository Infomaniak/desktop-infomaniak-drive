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
#include "theme.h"

#include <QDir>
#include <QHeaderView>
#include <QMutableListIterator>
#include <QScopedValueRollback>

namespace KDC {

static const int treeWidgetIndentation = 30;
static const int treeWidgetEditorMargin = 2;

static const QString commonDocumentsFolderName("Common documents");
static const QString sharedFolderName("Shared");

// 1st column roles
static const int viewIconPathRole = Qt::UserRole;
static const int dirRole = Qt::UserRole + 1;
static const int baseDirRole = Qt::UserRole + 2;
static const int sizeRole = Qt::UserRole + 3;

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
        if (path.endsWith(dirSeparator)) {
            path.chop(1);
        }
        parent->setData(TreeWidgetColumn::Folder, dirRole, path);
        parent->setData(TreeWidgetColumn::Folder, baseDirRole, path);
    }
    else {
        QString folderName = pathTrail.first();
        CustomTreeWidgetItem *item = static_cast<CustomTreeWidgetItem *>(findFirstChild(parent, folderName));
        if (!item) {
            item = new CustomTreeWidgetItem(parent);

            // Set icon
            setFolderIcon(item, folderName);

            // Set name
            item->setText(TreeWidgetColumn::Folder, folderName);

            // Set size
            if (size >= 0) {
                item->setData(TreeWidgetColumn::Folder, sizeRole, size);
            }

            item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        }

        pathTrail.removeFirst();
        insertPath(item, pathTrail, path, size);
    }
}

void BaseFolderTreeItemWidget::initUI()
{
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSortingEnabled(true);
    sortByColumn(TreeWidgetColumn::Folder, Qt::AscendingOrder);
    setItemDelegate(new CustomDelegate(this));
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

QString BaseFolderTreeItemWidget::iconPath(const QString &folderName)
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

QColor BaseFolderTreeItemWidget::iconColor(const QString &folderName)
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

void BaseFolderTreeItemWidget::setFolderIcon()
{
    if (_folderIconColor != QColor() && _folderIconSize != QSize()) {
        setIconSize(_folderIconSize);
        if (topLevelItem(0)) {
            setSubFoldersIcon(topLevelItem(0));
        }
    }
}

void BaseFolderTreeItemWidget::setFolderIcon(QTreeWidgetItem *item, const QString &folderName)
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
    QString folderPath = OCC::Theme::instance()->defaultServerFolder();
    return folderPath.startsWith(dirSeparator) ? folderPath.mid(1) : folderPath;
}

void BaseFolderTreeItemWidget::onUpdateDirectories(QStringList list)
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
        root->setData(TreeWidgetColumn::Folder, baseDirRole, QString());

        // Set size
        qint64 size = job ? job->_sizes.value(pathToRemove, -1) : -1;
        if (size >= 0) {
            root->setData(TreeWidgetColumn::Folder, sizeRole, size);
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
        folderPath.append(dirSeparator);
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
    if (current && !current->text(TreeWidgetColumn::Folder).isEmpty()) {
        // Add action icon
        current->setIcon(TreeWidgetColumn::Action,
                         OCC::Utility::getIconWithColor(":/client/resources/icons/actions/folder-add.svg", _addIconColor)
                         .pixmap(_addIconSize));

        _currentFolderPath = current->data(TreeWidgetColumn::Folder, dirRole).toString();
        QString currentFolderBasePath = current->data(TreeWidgetColumn::Folder, baseDirRole).toString();
        qint64 currentFolderSize = current->data(TreeWidgetColumn::Folder, sizeRole).toLongLong();
        emit folderSelected(_currentFolderPath, currentFolderBasePath, currentFolderSize);
    }

    if (previous) {
        if (previous->text(TreeWidgetColumn::Folder).isEmpty()) {
            // Remove item
            QTreeWidgetItem *parent = previous->parent();
            QTimer::singleShot(0, this, [=](){ parent->removeChild(previous); });
        }
        else {
            // Remove action icon
            previous->setIcon(TreeWidgetColumn::Action, QIcon());
        }
    }
}

void BaseFolderTreeItemWidget::onItemClicked(QTreeWidgetItem *item, int column)
{
    if (column == TreeWidgetColumn::Action && !item->text(TreeWidgetColumn::Folder).isEmpty()) {
        // Add Folder
        CustomTreeWidgetItem *newItem = new CustomTreeWidgetItem(item);

        // Set icon
        setFolderIcon(newItem, QString());

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
        QString parentPath = item->parent()->data(TreeWidgetColumn::Folder, dirRole).toString();
        QString path = parentPath + dirSeparator + item->text(TreeWidgetColumn::Folder);
        item->setData(TreeWidgetColumn::Folder, dirRole, path);
        QString parentBasePath = item->parent()->data(TreeWidgetColumn::Folder, baseDirRole).toString();
        item->setData(TreeWidgetColumn::Folder, baseDirRole, parentBasePath);
        item->setData(TreeWidgetColumn::Folder, sizeRole, 0);
        onCurrentItemChanged(item, nullptr);
        scrollToItem(item);

        if (item->text(TreeWidgetColumn::Folder).isEmpty()) {
            emit noFolderSelected();
        }
        else {
            emit folderSelected(path, parentBasePath, 0);
        }
    }
}

BaseFolderTreeItemWidget::CustomDelegate::CustomDelegate(BaseFolderTreeItemWidget *treeWidget, QObject *parent)
    : QStyledItemDelegate(parent)
    , _treeWidget(treeWidget)
{
}

void BaseFolderTreeItemWidget::CustomDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // Center editor into tree widget item
    QTreeWidgetItem *item = _treeWidget->itemFromIndex(index);
    QRect itemRect = _treeWidget->visualItemRect(item);

    QStyleOptionViewItem itemOption = option;
    initStyleOption(&itemOption, index);
    QRect lineEditRect = QApplication::style()->subElementRect(QStyle::SE_ItemViewItemText, &itemOption, editor);
    lineEditRect.setTop(itemRect.top() + treeWidgetEditorMargin);
    editor->setGeometry(lineEditRect);
}

}
