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

#include "customtreewidgetitem.h"
#include "accountinfo.h"
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

public:
    explicit FolderTreeItemWidget(const QString &folderId, bool displayRoot, QWidget *parent = nullptr);
    explicit FolderTreeItemWidget(const QString &accountId, const QString &folderPath, bool displayRoot, QWidget *parent = nullptr);

    inline void setAccountPtr(OCC::AccountPtr accountPtr) { _accountPtr = accountPtr; };
    void loadSubFolders();
    QStringList createBlackList(QTreeWidgetItem *root = 0) const;
    inline QString folderId() { return _folderId; }
    qint64 nodeSize(QTreeWidgetItem *item) const;

signals:
    void message(const QString &text);
    void showMessage(bool show);
    void needToSave();

private:
    enum TreeWidgetColumn {
        Folder = 0,
        Size
    };

    enum Mode {
        Creation = 0,
        Update
    };

    QString _folderId;
    QString _accountId;
    QString _folderPath;
    QStringList _oldBlackList;
    bool _displayRoot;
    Mode _mode;
    OCC::Folder *_currentFolder;
    ExcludedFiles _excludedFiles;
    OCC::AccountPtr _accountPtr;
    QColor _folderIconColor;
    QSize _folderIconSize;
    QColor _sizeTextColor;
    bool _inserting;

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

    void initUI();
    QString iconPath(const QString &folderName);
    QColor iconColor(const QString &folderName);
    void setFolderIcon();
    void setFolderIcon(QTreeWidgetItem *item, const QString &folderName);
    void setSubFoldersIcon(QTreeWidgetItem *parent);
    QTreeWidgetItem *findFirstChild(QTreeWidgetItem *parent, const QString &text);
    OCC::AccountPtr getAccountPtr();
    QString getFolderPath();
    void insertPath(QTreeWidgetItem *parent, QStringList pathTrail, QString path, qint64 size);

private slots:
    void onUpdateDirectories(QStringList list);
    void onLoadSubFoldersError(QNetworkReply *reply);
    void onItemExpanded(QTreeWidgetItem *item);
    void onItemChanged(QTreeWidgetItem *item, int col);
};

}

