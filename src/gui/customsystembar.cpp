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
#include "common/utility.h"

#include <QBoxLayout>
#include <QApplication>
#include <QIcon>
#include <QStyle>

namespace KDC {

static const int popupBarHeight = 38;
static const int popupBoxHMargin = 20;
static const int popupBoxVTMargin = 20;
static const int popupBoxVBMargin = 2;

static const int macDialogBarHeight = 22;
static const int macDialogBoxHMargin = 5;
static const int macDialogBoxVMargin = 3;
static const int macDialogBoxSpacing = 3;
static const QSize macIconSize = QSize(12, 12);

static const int winDialogBarHeight = 22;
static const int winDialogBoxHMargin = 18;
static const int winDialogBoxVMargin = 3;
static const QSize winIconSize = QSize(10, 10);

CustomSystemBar::CustomSystemBar(bool popup, QWidget *parent)
    : QWidget(parent)
    , _popup(popup)
    , _dragging(false)
    , _lastCursorPosition(QPoint())
{
    QHBoxLayout *hBox = new QHBoxLayout();
    setLayout(hBox);

    if (_popup) {
        setMinimumHeight(popupBarHeight);
        setMaximumHeight(popupBarHeight);
        hBox->setContentsMargins(popupBoxHMargin, popupBoxVTMargin, popupBoxHMargin, popupBoxVBMargin);

        CustomToolButton *exitButton = new CustomToolButton(this);
        exitButton->setObjectName("exitButton");
        exitButton->setIconPath(":/client/resources/icons/actions/close.svg");
        hBox->addStretch();
        hBox->addWidget(exitButton);

        connect(exitButton, &CustomToolButton::clicked, this, &CustomSystemBar::onExit);
    }
    else {
        QToolButton *closeButton = new QToolButton(this);

        if (OCC::Utility::isMac()) {
            setMinimumHeight(macDialogBarHeight);
            setMaximumHeight(macDialogBarHeight);
            hBox->setContentsMargins(macDialogBoxHMargin, macDialogBoxVMargin, macDialogBoxHMargin, macDialogBoxVMargin);
            hBox->setSpacing(macDialogBoxSpacing);

            QIcon closeIcon;
            closeIcon.addFile(":/client/resources/icons/mac/dark/close.svg", QSize(), QIcon::Normal);
            closeIcon.addFile(":/client/resources/icons/mac/dark/close-hover.svg", QSize(), QIcon::Active);
            closeIcon.addFile(":/client/resources/icons/mac/dark/unactive.svg", QSize(), QIcon::Disabled);
            closeButton->setIcon(closeIcon);
            closeButton->setIconSize(macIconSize);
            closeButton->setAutoRaise(true);
            hBox->addWidget(closeButton);

            QIcon minIcon;
            minIcon.addFile(":/client/resources/icons/mac/dark/unactive.svg", QSize(), QIcon::Disabled);
            QToolButton *minButton = new QToolButton(this);
            minButton->setIcon(minIcon);
            minButton->setIconSize(macIconSize);
            minButton->setEnabled(false);
            hBox->addWidget(minButton);

            QIcon maxIcon;
            maxIcon.addFile(":/client/resources/icons/mac/dark/unactive.svg", QSize(), QIcon::Disabled);
            QToolButton *maxButton = new QToolButton(this);
            maxButton->setIcon(maxIcon);
            maxButton->setIconSize(macIconSize);
            maxButton->setEnabled(false);
            hBox->addWidget(maxButton);
            hBox->addStretch();
        }
        else {
            setMinimumHeight(winDialogBarHeight);
            setMaximumHeight(winDialogBarHeight);
            hBox->setContentsMargins(winDialogBoxHMargin, winDialogBoxVMargin, winDialogBoxHMargin, winDialogBoxVMargin);

            QIcon closeIcon;
            closeIcon.addFile(":/client/resources/icons/windows/white/close.svg");
            closeButton->setIcon(closeIcon);
            closeButton->setIconSize(winIconSize);
            closeButton->setAutoRaise(true);
            hBox->addStretch();
            hBox->addWidget(closeButton);
        }

        connect(closeButton, &QToolButton::clicked, this, &CustomSystemBar::onExit);
    }
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

bool CustomSystemBar::event(QEvent *event)
{
    if (event->type() == QEvent::WindowActivate || event->type() == QEvent::WindowDeactivate) {
        QList<QToolButton *> buttonList = findChildren<QToolButton*>();
        for (QToolButton *button : buttonList) {
            button->setEnabled(event->type() == QEvent::WindowActivate ? true : false);
        }
    }
    return QWidget::event(event);
}

void CustomSystemBar::onExit(bool checked)
{
    Q_UNUSED(checked)

    emit exit();
}

}
