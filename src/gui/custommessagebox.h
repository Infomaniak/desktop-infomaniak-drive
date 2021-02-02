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

#include <QBoxLayout>
#include <QDialog>
#include <QLabel>
#include <QMessageBox>
#include <QSize>

namespace KDC {

class CustomMessageBox : public CustomDialog
{
    Q_OBJECT

    Q_PROPERTY(QSize icon_size READ iconSize WRITE setIconSize)

public:
    explicit CustomMessageBox(QMessageBox::Icon icon, const QString &text,
                              const QString &warningText, bool warning,
                              QMessageBox::StandardButtons buttons = QMessageBox::NoButton,
                              QWidget *parent = nullptr);

    explicit CustomMessageBox(QMessageBox::Icon icon, const QString &text,
                              QMessageBox::StandardButtons buttons = QMessageBox::NoButton,
                              QWidget *parent = nullptr);

    void addButton(const QString &text, QMessageBox::StandardButton button);
    void setDefaultButton(QMessageBox::StandardButton buttonType);

private:
    QMessageBox::Icon _icon;
    QLabel *_warningLabel;
    QLabel *_textLabel;
    QLabel *_iconLabel;
    QHBoxLayout *_buttonsHBox;
    QColor _backgroundColor;
    QSize _iconSize;
    int _buttonCount;

    inline QSize iconSize() const { return _iconSize; }
    inline void setIconSize(const QSize &size) {
        _iconSize = size;
        setIcon();
    }

    void setIcon();
    QSize sizeHint() const override;

private slots:
    void showEvent(QShowEvent *event) override;

    void onButtonClicked(bool checked = false);
    void onExit();
};

}



