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
#include "customcheckbox.h"

#include <QTableView>
#include <QStandardItemModel>

namespace KDC {

class FileExclusionDialog : public CustomDialog
{
    Q_OBJECT

    Q_PROPERTY(QColor action_icon_color READ actionIconColor WRITE setActionIconColor)
    Q_PROPERTY(QSize action_icon_size READ actionIconSize WRITE setActionIconSize)

public:
    explicit FileExclusionDialog(QWidget *parent = nullptr);

private:
    enum tableColumn {
        Pattern = 0,
        Deletable,
        Action
    };

    CustomCheckBox *_hiddenFilesCheckBox;
    QStandardItemModel *_filesTableModel;
    QTableView *_filesTableView;
    QPushButton *_saveButton;
    QColor _actionIconColor;
    QSize _actionIconSize;
    bool _needToSave;

    inline QColor actionIconColor() const { return _actionIconColor; }
    inline QSize actionIconSize() const { return _actionIconSize; }

    void initUI();
    void updateUI();
    void readIgnoreFile(const QString &file, bool global);
    void addPattern(const QString &pattern, bool deletable, bool readOnly, bool global,
        const QStringList &skippedLines = QStringList());
    void setActionIconColor(const QColor &color);
    void setActionIconSize(const QSize &size);
    void setActionIcon();
    void setActionIcon(QStandardItem *item, const QString &viewIconPath);
    void setNeedToSave(bool value);

private slots:
    void onExit();
    void onHiddenFilesCheckBoxClicked(bool checked = false);
    void onAddFileButtonTriggered(bool checked = false);
    void onTableViewClicked(const QModelIndex &index);
    void onDeletableCheckBoxClicked(bool checked = false);
    void onSaveButtonTriggered(bool checked = false);
};

}
