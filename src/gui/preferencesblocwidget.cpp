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

#include "preferencesblocwidget.h"

#include <QPainter>

namespace KDC {

static const int cornerRadius = 10;
static const int boxHMargin= 15;
static const int boxVMargin = 18;
static const int boxSpacing = 20;

PreferencesBlocWidget::PreferencesBlocWidget(QWidget *parent)
    : QWidget(parent)
    , _layout(nullptr)
{
    setContentsMargins(0, 0, 0, 0);

    _layout = new QVBoxLayout();
    _layout->setContentsMargins(boxHMargin, boxVMargin, boxHMargin, boxVMargin);
    _layout->setSpacing(boxSpacing);
    setLayout(_layout);
}

void PreferencesBlocWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    // Draw background
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);

    // Draw round rectangle
    QPainterPath painterPath;
    painterPath.addRoundedRect(rect(), cornerRadius, cornerRadius);

    painter.setBrush(backgroundColor());
    painter.drawPath(painterPath);
}

}
