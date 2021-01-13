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
#include "foldertreeitemwidget.h"
#include "accountinfo.h"
#include "folderman.h"

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

public:
    explicit ServerFoldersDialog(const QString &accountId, const QString &serverFolderName,
                                 const QString &serverFolderPath, QWidget *parent = nullptr);

    qint64 selectionSize() const;
    QStringList createBlackList() const;

private:
    QString _accountId;
    const QString _serverFolderName;
    const QString _serverFolderPath;
    OCC::Folder *_currentFolder;
    FolderTreeItemWidget *_folderTreeItemWidget;
    QPushButton *_backButton;
    QPushButton *_continueButton;
    bool _needToSave;

    void setButtonIcon(const QColor &value) override;

    void initUI();
    void updateUI();

private slots:
    void onExit();
    void onBackButtonTriggered(bool checked = false);
    void onContinueButtonTriggered(bool checked = false);
    void onSubfoldersLoaded(bool error, bool empty);
    void onNeedToSave();
};

}

