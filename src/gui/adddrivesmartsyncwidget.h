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

class AddDriveSmartSyncWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor logo_color READ logoColor WRITE setLogoColor)

public:
    explicit AddDriveSmartSyncWidget(QWidget *parent = nullptr);

    void setButtonIcon(const QColor &value);

    inline bool smartSync() const { return _smartSync; }

signals:
    void terminated(bool next = true);

private:
    QLabel *_logoTextIconLabel;
    QPushButton *_backButton;
    QPushButton *_laterButton;
    QPushButton *_yesButton;
    QColor _logoColor;
    bool _smartSync;

    inline QColor logoColor() const { return _logoColor; }
    void setLogoColor(const QColor& color);

    void initUI();

private slots:
    void onBackButtonTriggered(bool checked = false);
    void onLaterButtonTriggered(bool checked = false);
    void onYesButtonTriggered(bool checked = false);
};

}

