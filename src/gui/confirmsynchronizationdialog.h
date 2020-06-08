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

#include <QLabel>
#include <QPushButton>

namespace KDC {

class ConfirmSynchronizationDialog : public CustomDialog
{
    Q_OBJECT

    Q_PROPERTY(QColor arrow_icon_color READ arrowIconColor WRITE setArrowIconColor)

public:
    explicit ConfirmSynchronizationDialog(const QString &localFolderName, qint64 localFolderSize,
                                          const QString &serverFolderName, qint64 serverFolderSize,
                                          QWidget *parent = nullptr);

private:
    QString _localFolderName;
    qint64 _localFolderSize;
    QString _serverFolderName;
    qint64 _serverFolderSize;
    QLabel *_leftArrowIconLabel;
    QLabel *_rightArrowIconLabel;
    QPushButton *_continueButton;
    QColor _arrowIconColor;

    inline QColor arrowIconColor() const { return _arrowIconColor; }
    inline void setArrowIconColor(QColor color) {
        _arrowIconColor = color;
        setArrowIcon();
    }

    void initUI();
    void setArrowIcon();

private slots:
    void onExit();
    void onBackButtonTriggered(bool checked = false);
    void onContinueButtonTriggered(bool checked = false);
};

}

