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
#include "csync_exclude.h"

#include <QColor>
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

public:
    explicit ServerFoldersDialog(const AccountInfo *accountInfo, QWidget *parent = nullptr);

signals:
    void infoIconColorChanged();
    void infoIconSizeChanged();

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

    const AccountInfo *_accountInfo;
    QString _currentFolderId;
    QString _currentFolderPath;
    bool _inserting;
    ExcludedFiles _excludedFiles;
    QLabel *_infoIconLabel;
    QLabel *_availableSpaceTextLabel;
    QTreeWidget *_folderTreeWidget;
    QPushButton *_saveButton;
    QColor _infoIconColor;
    QSize _infoIconSize;
    bool _needToSave;

    inline QColor infoIconColor() const { return _infoIconColor; }
    inline void setInfoIconColor(QColor color) {
        _infoIconColor = color;
        emit infoIconColorChanged();
    }

    inline QSize infoIconSize() const { return _infoIconSize; }
    inline void setInfoIconSize(QSize size) {
        _infoIconSize = size;
        emit infoIconSizeChanged();
    }

    void setInfoIcon();
    void initUI();
    void updateUI();
    void refreshFolders();

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

