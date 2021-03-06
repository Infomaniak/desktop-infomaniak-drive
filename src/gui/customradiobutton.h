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

#include "customtooltip.h"

#include <QEvent>
#include <QRadioButton>
#include <QString>

namespace KDC {

class CustomRadioButton : public QRadioButton
{
    Q_OBJECT

public:
    explicit CustomRadioButton(QWidget *parent = nullptr);

    inline void setToolTip(const QString &text) { _toolTipText = text; }

private:
    QString _toolTipText;
    int _toolTipDuration;
    CustomToolTip *_customToolTip;

    bool event(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

private slots:
    void onClicked(bool checked = false);
};

}

