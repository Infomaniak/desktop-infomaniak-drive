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

#include "synchronizeditem.h"
#include "customtoolbutton.h"

#include <QColor>
#include <QIcon>
#include <QLabel>
#include <QPaintEvent>
#include <QSize>
#include <QWidget>

namespace KDC {

class SynchronizedItemWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QSize file_icon_size READ fileIconSize WRITE setFileIconSize)
    Q_PROPERTY(QSize direction_icon_size READ directionIconSize WRITE setDirectionIconSize)
    Q_PROPERTY(QColor direction_icon_color READ directionIconColor WRITE setDirectionIconColor)
    Q_PROPERTY(QColor background_color READ backgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(QColor background_color_selection READ backgroundColorSelection WRITE setBackgroundColorSelection)

public:
    explicit SynchronizedItemWidget(const SynchronizedItem &item, QWidget *parent = nullptr);

    inline QSize fileIconSize() const { return _fileIconSize; }
    inline void setFileIconSize(const QSize &size) {
        _fileIconSize = size;
        emit fileIconSizeChanged();
    }

    inline QSize directionIconSize() const { return _directionIconSize; }
    inline void setDirectionIconSize(const QSize &size) {
        _directionIconSize = size;
        emit directionIconSizeChanged();
    }

    inline QColor directionIconColor() const { return _directionIconColor; }
    inline void setDirectionIconColor(const QColor &color) {
        _directionIconColor = color;
        emit directionIconColorChanged();
    }

    inline QColor backgroundColor() const { return _backgroundColor; }
    inline void setBackgroundColor(const QColor &value) { _backgroundColor = value; }

    inline QColor backgroundColorSelection() const { return _backgroundColorSelection; }
    inline void setBackgroundColorSelection(const QColor &value) { _backgroundColorSelection = value; }

    bool isSelected() const { return _isSelected; };
    void setSelected(bool isSelected);

    inline const SynchronizedItem *item() const { return &_item; };

signals:
    void fileIconSizeChanged();
    void directionIconSizeChanged();
    void directionIconColorChanged();
    void openFolder();
    void open();
    void addToFavourites();
    void manageRightAndSharing();
    void copyLink();
    void displayOnWebview();

private:
    SynchronizedItem _item;
    bool _isSelected;
    QSize _fileIconSize;
    QSize _directionIconSize;
    QColor _directionIconColor;
    QColor _backgroundColor;
    QColor _backgroundColorSelection;
    QLabel *_fileIconLabel;
    QLabel *_fileDirectionLabel;
    CustomToolButton *_folderButton;
    CustomToolButton *_menuButton;

    void paintEvent(QPaintEvent* event) override;
    bool event(QEvent *event) override;

    QString getFileIconPathFromFileName(const QString &fileName) const;
    QString getStatusIconPathFromStatus(OCC::SyncFileItem::Status status) const;
    QIcon getIconWithStatus(const QString &fileName, OCC::SyncFileItem::Status status);
    void setDirectionIcon();

private slots:
    void onFileIconSizeChanged();
    void onDirectionIconSizeChanged();
    void onDirectionIconColorChanged();
    void onFolderButtonClicked();
    void onMenuButtonClicked();
    void onOpenActionTriggered(bool checked = false);
    void onFavoritesActionTriggered(bool checked = false);
    void onRightAndSharingActionTriggered(bool checked = false);
    void onCopyLinkActionTriggered(bool checked = false);
    void onDisplayOnDriveActionTriggered(bool checked = false);
};

}
