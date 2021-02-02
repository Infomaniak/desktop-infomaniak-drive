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

#include "account.h"

#include <QColor>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QSize>

namespace KDC {

class AddDriveLocalFolderWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor folder_icon_color READ folderIconColor WRITE setFolderIconColor)
    Q_PROPERTY(QSize folder_icon_size READ folderIconSize WRITE setFolderIconSize)
    Q_PROPERTY(QColor info_icon_color READ infoIconColor WRITE setInfoIconColor)
    Q_PROPERTY(QSize info_icon_size READ infoIconSize WRITE setInfoIconSize)
    Q_PROPERTY(QSize warning_icon_size READ warningIconSize WRITE setWarningIconSize)
    Q_PROPERTY(QColor warning_icon_color READ warningIconColor WRITE setWarningIconColor)
    Q_PROPERTY(QColor logo_color READ logoColor WRITE setLogoColor)

public:
    explicit AddDriveLocalFolderWidget(QWidget *parent = nullptr);

    void setAccountPtr(OCC::AccountPtr accountPtr);
    void setLocalFolderPath(const QString &path);
    inline QString localFolderPath() const { return _localFolderPath; }
    inline void setSmartSync(bool smartSync) { _smartSync = smartSync; }
    inline bool folderCompatibleWithSmartSync() const { return _folderCompatibleWithSmartSync; }
    void setButtonIcon(const QColor &value);

signals:
    void terminated(bool next = true);

private:
    QString _localFolderPath;
    QString _defaultLocalFolderPath;
    QLabel *_logoTextIconLabel;
    QLabel *_titleLabel;
    QLabel *_folderIconLabel;
    QLabel *_folderNameLabel;
    QLabel *_folderPathLabel;
    QWidget *_warningWidget;
    QLabel *_warningIconLabel;
    QLabel *_warningLabel;
    QWidget *_infoWidget;
    QLabel *_infoIconLabel;
    QLabel *_infoLabel;
    QPushButton *_backButton;
    QPushButton *_endButton;
    QColor _folderIconColor;
    QSize _folderIconSize;
    QColor _infoIconColor;
    QSize _infoIconSize;
    QColor _warningIconColor;
    QSize _warningIconSize;
    QColor _logoColor;
    bool _needToSave;
    bool _smartSync;
    bool _folderCompatibleWithSmartSync;

    inline QColor folderIconColor() const { return _folderIconColor; }
    inline void setFolderIconColor(QColor color)
    {
        _folderIconColor = color;
        setFolderIcon();
    }

    inline QSize folderIconSize() const { return _folderIconSize; }
    inline void setFolderIconSize(QSize size)
    {
        _folderIconSize = size;
        setFolderIcon();
    }

    inline QColor infoIconColor() const { return _infoIconColor; }
    inline void setInfoIconColor(QColor color)
    {
        _infoIconColor = color;
        setInfoIcon();
    }

    inline QSize infoIconSize() const { return _infoIconSize; }
    inline void setInfoIconSize(QSize size)
    {
        _infoIconSize = size;
        setInfoIcon();
    }

    inline QSize warningIconSize() const { return _warningIconSize; }
    inline void setWarningIconSize(const QSize &size) {
        _warningIconSize = size;
        setWarningIcon();
    }

    inline QColor warningIconColor() const { return _warningIconColor; }
    inline void setWarningIconColor(const QColor &value) {
        _warningIconColor = value;
        setWarningIcon();
    }

    inline QColor logoColor() const { return _logoColor; }
    void setLogoColor(const QColor& color);

    void initUI();
    void updateUI();
    void selectFolder(const QString &startDirPath);
    void setFolderIcon();
    void setInfoIcon();
    void setWarningIcon();

private slots:
    void onDisplayMessage(const QString &text);
    void onNeedToSave();
    void onBackButtonTriggered(bool checked = false);
    void onContinueButtonTriggered(bool checked = false);
    void onUpdateFolderButtonTriggered(bool checked = false);
    void onLinkActivated(const QString &link);
};

}

