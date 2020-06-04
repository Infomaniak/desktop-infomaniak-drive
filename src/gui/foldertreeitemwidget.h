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

#pragma once

#include "folderman.h"
#include "csync_exclude.h"

#include <QColor>
#include <QTreeWidget>
#include <QSize>

namespace KDC {

class FolderTreeItemWidget : public QTreeWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor folder_icon_color READ folderIconColor WRITE setFolderIconColor)
    Q_PROPERTY(QSize folder_icon_size READ folderIconSize WRITE setFolderIconSize)
    Q_PROPERTY(QColor size_text_color READ sizeTextColor WRITE setSizeTextColor)
    Q_PROPERTY(int header_font_weight READ headerFontWeight WRITE setHeaderFontWeight)

public:
    explicit FolderTreeItemWidget(const QString &folderId, bool displayRoot, QWidget *parent = nullptr);

    void loadSubFolders();
    QStringList createBlackList(QTreeWidgetItem *root = 0) const;

signals:
    void message(const QString &text);
    void showMessage(bool show);
    void needToSave();

private:
    class TreeViewItem : public QTreeWidgetItem
    {
    public:
        TreeViewItem(int type = QTreeWidgetItem::Type)
            : QTreeWidgetItem(type)
        {
        }

        TreeViewItem(const QStringList &strings, int type = QTreeWidgetItem::Type)
            : QTreeWidgetItem(strings, type)
        {
        }

        TreeViewItem(QTreeWidget *view, int type = QTreeWidgetItem::Type)
            : QTreeWidgetItem(view, type)
        {
        }

        TreeViewItem(QTreeWidgetItem *parent, int type = QTreeWidgetItem::Type)
            : QTreeWidgetItem(parent, type)
        {
        }

    private:
        bool operator<(const QTreeWidgetItem &other) const
        {
            int column = treeWidget()->sortColumn();
            if (column == 1) {
                return data(1, Qt::UserRole).toLongLong() < other.data(1, Qt::UserRole).toLongLong();
            }
            return QTreeWidgetItem::operator<(other);
        }
    };

    enum TreeWidgetColumn {
        Folder = 0,
        Size
    };

    QString _folderId;
    bool _displayRoot;
    OCC::Folder *_currentFolder;
    ExcludedFiles _excludedFiles;
    QColor _folderIconColor;
    QSize _folderIconSize;
    QColor _sizeTextColor;
    int _headerFontWeight;
    bool _inserting;
    QStringList _oldBlackList;

    inline QColor folderIconColor() const { return _folderIconColor; }
    inline void setFolderIconColor(QColor color) {
        _folderIconColor = color;
        setFolderIcon();
    }

    inline QSize folderIconSize() const { return _folderIconSize; }
    inline void setFolderIconSize(QSize size) {
        _folderIconSize = size;
        setFolderIcon();
    }

    inline QColor sizeTextColor() const { return _sizeTextColor; }
    inline void setSizeTextColor(QColor color) { _sizeTextColor = color; }

    inline int headerFontWeight() const { return _headerFontWeight; }
    inline void setHeaderFontWeight(int headerFontWeight) { _headerFontWeight = headerFontWeight; }

    void setFolderIcon();
    void setFolderIcon(QTreeWidgetItem *item, const QString &viewIconPath);
    void setSubFoldersIcon(QTreeWidgetItem *parent);
    QTreeWidgetItem *findFirstChild(QTreeWidgetItem *parent, const QString &text);
    OCC::AccountPtr getAccountPtr();
    QString getFolderPath();
    void insertPath(QTreeWidgetItem *parent, QStringList pathTrail, QString path, qint64 size);

private slots:
    void slotUpdateDirectories(QStringList list);
    void slotLscolFinishedWithError(QNetworkReply *reply);
    void slotItemExpanded(QTreeWidgetItem *item);
    void slotItemChanged(QTreeWidgetItem *item, int col);
};

}

