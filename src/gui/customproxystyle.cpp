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

#include "customproxystyle.h"

#include <QVariant>

namespace KDC {

static const int tooltipWakeUpDelay = 200; // ms

const char CustomProxyStyle::focusRectangleProperty[] = "focusRectangle";

int CustomProxyStyle::styleHint(QStyle::StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    if (hint == QStyle::SH_ToolTip_WakeUpDelay) {
        return tooltipWakeUpDelay;
    }
    return QProxyStyle::styleHint(hint, option, widget, returnData);
}

void CustomProxyStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    // No focus rectangle
    if (element == QStyle::PE_FrameFocusRect && !widget->property(focusRectangleProperty).toBool()) {
        return;
    }

    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

}
