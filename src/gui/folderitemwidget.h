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

#include "folderinfo.h"
#include "customtoolbutton.h"

#include <QLabel>
#include <QString>
#include <QWidget>

namespace KDC {

class FolderItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FolderItemWidget(const QString &folderId, const FolderInfo *folderInfo, QWidget *parent = nullptr);

    inline QString folderId() const { return _folderId; };
    void updateItem(const FolderInfo *folderInfo);
    void setUpdateWidgetVisible(bool visible);
    void setSmartSync(bool smartSync);
    void setFolderCompatibleWithSmartSync(bool folderCompatibleWithSmartSync);

signals:
    void runSync(const QString &folderId);
    void pauseSync(const QString &folderId);
    void resumeSync(const QString &folderId);
    void unSync(const QString &folderId);
    void displayFolderDetail(const QString &folderId, bool display);
    void displayFolderDetailCanceled();
    void openFolder(const QString &filePath);
    void cancelUpdate(const QString &folderId);
    void validateUpdate(const QString &folderId);

private:
    const QString _folderId;
    const FolderInfo *_folderInfo;
    CustomToolButton *_expandButton;
    CustomToolButton *_menuButton;
    QLabel *_statusIconLabel;
    QLabel *_nameLabel;
    QLabel *_smartSyncIconLabel;
    QWidget *_updateWidget;
    bool _isExpanded;
    bool _smartSync;
    bool _folderCompatibleWithSmartSync;

    void setExpandButton();

private slots:
    void onMenuButtonClicked();
    void onExpandButtonClicked();
    void onCancelButtonClicked();
    void onValidateButtonClicked();
    void onOpenFolder(const QString &link);
    void onSyncTriggered();
    void onPauseTriggered();
    void onResumeTriggered();
    void onUnsyncTriggered();
    void onDisplayFolderDetailCanceled();
};

}

