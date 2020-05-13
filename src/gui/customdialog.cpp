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

#include "customdialog.h"
#include "customsystembar.h"
#include "guiutility.h"

#include <QBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>

namespace KDC {

static const int cornerRadius = 5;
static const int hMargin= 20;
static const int vMargin = 5;
static const int shadowBlurRadius = 20;
static const int boxHMargin= 40;
static const int boxVTMargin = 0;
static const int boxVBMargin = 40;
static const int boxSpacing = 15;

CustomDialog::CustomDialog(QWidget *parent)
    : QDialog(parent)
    , _backgroundColor(QColor())
    , _layout(nullptr)
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::X11BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground);

    setContentsMargins(hMargin, vMargin, hMargin, vMargin);

    QVBoxLayout *mainVBox = new QVBoxLayout();
    mainVBox->setContentsMargins(0, 0, 0, 0);
    mainVBox->setSpacing(0);
    setLayout(mainVBox);

    // System bar
    CustomSystemBar *systemBar = new CustomSystemBar(this);
    mainVBox->addWidget(systemBar);

    _layout = new QVBoxLayout();
    _layout->setContentsMargins(boxHMargin, boxVTMargin, boxHMargin, boxVBMargin);
    _layout->setSpacing(boxSpacing);
    mainVBox->addLayout(_layout);
    mainVBox->setStretchFactor(_layout, 1);

    // Shadow
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
    effect->setBlurRadius(shadowBlurRadius);
    effect->setOffset(0);
    setGraphicsEffect(effect);

    connect(systemBar, &CustomSystemBar::drag, this, &CustomDialog::onDrag);
    connect(systemBar, &CustomSystemBar::exit, this, &CustomDialog::onExit);
}

void CustomDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    // Update shadow color
    QGraphicsDropShadowEffect *effect = qobject_cast<QGraphicsDropShadowEffect *>(graphicsEffect());
    if (effect) {
        effect->setColor(OCC::Utility::getShadowColor());
    }

    // Draw round rectangle
    QPainterPath painterPath;
    painterPath.addRoundedRect(rect().marginsRemoved(QMargins(hMargin, vMargin, hMargin, vMargin)),
                               cornerRadius, cornerRadius);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(backgroundColor());
    painter.drawPath(painterPath);
}

void CustomDialog::onDrag(const QPoint &move)
{
    this->move(pos() + move);
}

void CustomDialog::onExit()
{
    emit exit();
}

}
