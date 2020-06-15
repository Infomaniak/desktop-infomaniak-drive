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

class AddDriveServerFoldersWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor info_icon_color READ infoIconColor WRITE setInfoIconColor)
    Q_PROPERTY(QSize info_icon_size READ infoIconSize WRITE setInfoIconSize)
    Q_PROPERTY(QColor logo_color READ logoColor WRITE setLogoColor)

public:
    explicit AddDriveServerFoldersWidget(QWidget *parent = nullptr);

    void setAccountPtr(OCC::AccountPtr accountPtr);
    qint64 selectionSize() const;
    QStringList createBlackList() const;
    void setButtonIcon(const QColor &value);

signals:
    void terminated(bool next = true);

private:
    OCC::Folder *_currentFolder;
    QLabel *_logoTextIconLabel;
    QLabel *_infoIconLabel;
    QLabel *_availableSpaceTextLabel;
    FolderTreeItemWidget *_folderTreeItemWidget;
    QPushButton *_backButton;
    QPushButton *_continueButton;
    QColor _infoIconColor;
    QSize _infoIconSize;
    QColor _logoColor;
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

    inline QColor logoColor() const { return _logoColor; }
    void setLogoColor(const QColor& color);

    void initUI();
    void updateUI();
    void setInfoIcon();

private slots:
    void onSubfoldersLoaded(bool error, bool empty = false);
    void onNeedToSave();
    void onBackButtonTriggered(bool checked = false);
    void onContinueButtonTriggered(bool checked = false);
};

}

