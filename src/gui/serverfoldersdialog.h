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

#include "customdialog.h"
#include "accountinfo.h"
#include "folderman.h"
#include "csync_exclude.h"

#include <QColor>
#include <QIcon>
#include <QLabel>
#include <QNetworkReply>
#include <QPushButton>
#include <QSize>
#include <QStringList>
#include <QTreeWidget>

namespace KDC {

class ServerFoldersDialog : public CustomDialog
{
    Q_OBJECT

    Q_PROPERTY(QColor info_icon_color READ infoIconColor WRITE setInfoIconColor)
    Q_PROPERTY(QSize info_icon_size READ infoIconSize WRITE setInfoIconSize)
    Q_PROPERTY(QColor folder_icon_color READ folderIconColor WRITE setFolderIconColor)
    Q_PROPERTY(QSize folder_icon_size READ folderIconSize WRITE setFolderIconSize)
    Q_PROPERTY(int header_font_weight READ headerFontWeight WRITE setHeaderFontWeight)
    Q_PROPERTY(QColor size_text_color READ sizeTextColor WRITE setSizeTextColor)

public:
    explicit ServerFoldersDialog(const AccountInfo *accountInfo, QWidget *parent = nullptr);

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

    const AccountInfo *_accountInfo;
    OCC::Folder *_currentFolder;
    QStringList _oldBlackList;
    bool _inserting;
    ExcludedFiles _excludedFiles;
    QLabel *_infoIconLabel;
    QLabel *_availableSpaceTextLabel;
    QLabel *_messageLabel;
    QTreeWidget *_folderTreeWidget;
    QPushButton *_saveButton;
    QColor _infoIconColor;
    QSize _infoIconSize;
    QColor _folderIconColor;
    QSize _folderIconSize;
    int _headerFontWeight;
    QColor _sizeTextColor;
    bool _needToSave;

    inline QColor infoIconColor() const { return _infoIconColor; }
    inline void setInfoIconColor(QColor color) {
        _infoIconColor = color;
        setInfoIcon();
    }

    inline QSize infoIconSize() const { return _infoIconSize; }
    inline void setInfoIconSize(QSize size) {
        _infoIconSize = size;
        setInfoIcon();
    }

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

    inline int headerFontWeight() const { return _headerFontWeight; }
    inline void setHeaderFontWeight(int headerFontWeight) { _headerFontWeight = headerFontWeight; }

    inline QColor sizeTextColor() const { return _sizeTextColor; }
    inline void setSizeTextColor(QColor color) { _sizeTextColor = color; }

    void initUI();
    void updateUI();
    void setInfoIcon();
    void setFolderIcon();
    void setFolderIcon(QTreeWidgetItem *item, const QString &viewIconPath);
    void setFolderIconSubFolders(QTreeWidgetItem *parent);
    void setNeedToSave(bool value);
    QTreeWidgetItem *findFirstChild(QTreeWidgetItem *parent, const QString &text);
    void recursiveInsert(QTreeWidgetItem *parent, QStringList pathTrail, QString path, qint64 size);
    qint64 estimatedSize(QTreeWidgetItem *root = 0);

private slots:
    void onInfoIconColorChanged();
    void onInfoIconSizeChanged();
    void onExit();
    void onSaveButtonTriggered(bool checked = false);
    void slotUpdateDirectories(QStringList list);
    void slotItemExpanded(QTreeWidgetItem *item);
    void slotItemChanged(QTreeWidgetItem *item, int col);
    void slotLscolFinishedWithError(QNetworkReply *reply);
};

}

