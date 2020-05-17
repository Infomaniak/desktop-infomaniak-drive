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

#include "customsystembar.h"
#include "customtoolbutton.h"

#include <QBoxLayout>

namespace KDC {

static const int boxHMargin= 20;
static const int boxVTMargin = 20;
static const int boxVBMargin = 2;

CustomSystemBar::CustomSystemBar(QWidget *parent)
    : QWidget(parent)
    , _dragging(false)
    , _lastCursorPosition(QPoint())
{
    QHBoxLayout *hBox = new QHBoxLayout();
    hBox->setContentsMargins(boxHMargin, boxVTMargin, boxHMargin, boxVBMargin);
    setLayout(hBox);

    CustomToolButton *exitButton = new CustomToolButton(this);
    exitButton->setObjectName("exitButton");
    exitButton->setIconPath(":/client/resources/icons/actions/close.svg");
    hBox->addStretch();
    hBox->addWidget(exitButton);

    connect(exitButton, &CustomToolButton::clicked, this, &CustomSystemBar::onExit);
}

void CustomSystemBar::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    _dragging = true;
    _lastCursorPosition = QCursor::pos();
}

void CustomSystemBar::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
    if (_dragging) {
        _dragging = false;
        _lastCursorPosition = QPoint();
    }
}

void CustomSystemBar::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
    if (_dragging) {
        QPoint newCursorPosition = QCursor::pos();
        emit drag(newCursorPosition - _lastCursorPosition);
        _lastCursorPosition = newCursorPosition;
    }
}

void CustomSystemBar::onExit(bool checked)
{
    Q_UNUSED(checked)

    emit exit();
}

}
