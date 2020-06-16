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

#include <QColor>
#include <QLabel>
#include <QPushButton>
#include <QSize>

namespace KDC {

class LocalFolderDialog : public CustomDialog
{
    Q_OBJECT

    Q_PROPERTY(QColor folder_icon_color READ folderIconColor WRITE setFolderIconColor)
    Q_PROPERTY(QSize folder_icon_size READ folderIconSize WRITE setFolderIconSize)

public:
    explicit LocalFolderDialog(const QString &localFolderPath, QWidget *parent = nullptr);

    inline QString localFolderPath() const { return _localFolderPath; }

signals:
    void openFolder(const QString &filePath);

private:
    QString _localFolderPath;
    QPushButton *_continueButton;
    QWidget *_folderSelectionWidget;
    QWidget *_folderSelectedWidget;
    QLabel *_folderIconLabel;
    QLabel *_folderNameLabel;
    QLabel *_folderPathLabel;
    QColor _folderIconColor;
    QSize _folderIconSize;
    bool _okToContinue;

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

    void initUI();
    void updateUI();
    void setOkToContinue(bool value);
    void selectFolder(const QString &startDirPath);
    void setFolderIcon();

private slots:
    void onExit();
    void onContinueButtonTriggered(bool checked = false);
    void onSelectFolderButtonTriggered(bool checked = false);
    void onUpdateFolderButtonTriggered(bool checked = false);
    void onLinkActivated(const QString &link);
};

}

