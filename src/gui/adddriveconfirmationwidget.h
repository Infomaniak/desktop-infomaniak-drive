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

#include "guiutility.h"
#include "account.h"

#include <QColor>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QSize>

namespace KDC {

class AddDriveConfirmationWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor logo_color READ logoColor WRITE setLogoColor)

public:
    explicit AddDriveConfirmationWidget(QWidget *parent = nullptr);

    void setFolderPath(const QString &path);
    inline OCC::Utility::WizardAction action() { return _action; }

signals:
    void terminated(bool next = true);

private:
    QLabel *_logoTextIconLabel;
    QLabel *_descriptionLabel;
    QColor _logoColor;
    OCC::Utility::WizardAction _action;

    inline QColor logoColor() const { return _logoColor; }
    void setLogoColor(const QColor& color);

    void initUI();

private slots:
    void onOpenFoldersButtonTriggered(bool checked = false);
    void onOpenParametersButtonTriggered(bool checked = false);
    void onAddDriveButtonTriggered(bool checked = false);
};

}

