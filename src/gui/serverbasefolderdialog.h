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
#include "basefoldertreeitemwidget.h"
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

class ServerBaseFolderDialog : public CustomDialog
{
    Q_OBJECT

    Q_PROPERTY(QColor info_icon_color READ infoIconColor WRITE setInfoIconColor)
    Q_PROPERTY(QSize info_icon_size READ infoIconSize WRITE setInfoIconSize)

public:
    explicit ServerBaseFolderDialog(const QString &accountId, const QString &localFolderName, QWidget *parent = nullptr);

    inline QString serverFolderPath() const { return _serverFolderPath; }
    inline QString serverFolderBasePath() const { return _serverFolderBasePath; }
    inline qint64 selectionSize() const { return _serverFolderSize; };

private:
    QString _accountId;
    QString _localFolderName;
    OCC::Folder *_currentFolder;
    QLabel *_infoIconLabel;
    QLabel *_availableSpaceTextLabel;
    BaseFolderTreeItemWidget *_folderTreeItemWidget;
    QPushButton *_backButton;
    QPushButton *_continueButton;
    QColor _infoIconColor;
    QSize _infoIconSize;
    bool _okToContinue;
    QString _serverFolderPath;
    QString _serverFolderBasePath;
    qint64 _serverFolderSize;

    void setButtonIcon(const QColor &value) override;

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

    void initUI();
    void updateUI();
    void setInfoIcon();
    void setOkToContinue(bool value);

private slots:
    void onExit();
    void onBackButtonTriggered(bool checked = false);
    void onContinueButtonTriggered(bool checked = false);
    void onDisplayMessage(const QString &text);
    void onFolderSelected(const QString &folderPath, const QString &folderBasePath, qint64 folderSize);
    void onNoFolderSelected();
};

}

