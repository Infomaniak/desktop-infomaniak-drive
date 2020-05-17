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

#include "customswitch.h"

namespace KDC {

CustomSwitch::CustomSwitch(QWidget *parent)
    : CustomCheckBox(parent)
{
    setStyleSheet("QCheckBox::indicator:checked { image: url(:/client/resources/icons/actions/switch-on.svg); }"
                  "QCheckBox::indicator:unchecked { image: url(:/client/resources/icons/actions/switch-off.svg); }"
                  "QCheckBox::indicator:checked:disabled { image: url(:/client/resources/icons/actions/switch-disabled.svg); }"
                  "QCheckBox::indicator:unchecked:disabled { image: url(:/client/resources/icons/actions/switch-disabled.svg); }");
}

}
