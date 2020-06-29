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
#include <QStylePainter>
#include <QTableView>
#include <QTimer>
#include <QVariant>

namespace KDC {

static const QSize windowSize(625, 530);
static const int cornerRadius = 5;
static const int hMargin= 20;
static const int vMargin = 20;
static const int mainBoxHMargin= 0;
static const int mainBoxVTMargin = 0;
static const int mainBoxVBMargin = 40;
static const int shadowBlurRadius = 20;

CustomDialog::CustomDialog(bool popup, QWidget *parent)
    : QDialog(parent)
    , _backgroundColor(QColor())
    , _buttonIconColor(QColor())
    , _backgroundForcedColor(QColor())
    , _layout(nullptr)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    setMinimumSize(windowSize);
    setMaximumSize(windowSize);

    setContentsMargins(hMargin, vMargin, hMargin, vMargin);

    QVBoxLayout *mainVBox = new QVBoxLayout();
    mainVBox->setContentsMargins(0, 0, 0, 0);
    mainVBox->setSpacing(0);
    setLayout(mainVBox);

    // System bar
    CustomSystemBar *systemBar = nullptr;
    systemBar = new CustomSystemBar(popup, this);
    mainVBox->addWidget(systemBar);

    _layout = new QVBoxLayout();
    if (popup) {
        _layout->setContentsMargins(mainBoxHMargin, mainBoxVTMargin, mainBoxHMargin, mainBoxVBMargin);
        _layout->setSpacing(0);
    }

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

int CustomDialog::exec()
{
    return QDialog::exec();
}

int CustomDialog::exec(QPoint position)
{
    move(position);
    return exec();
}

void CustomDialog::forceRedraw()
{
#ifdef Q_OS_WINDOWS
    // Windows hack
    QTimer::singleShot(0, this, [=]()
    {
        if (geometry().height() > windowSize.height()) {
            setGeometry(geometry() + QMargins(0, 0, 0, -1));
            setMaximumHeight(windowSize.height());
        }
        else {
            setMaximumHeight(windowSize.height() + 1);
            setGeometry(geometry() + QMargins(0, 0, 0, 1));
        }
    });
#endif
}

void CustomDialog::setBackgroundForcedColor(const QColor &value)
{
    _backgroundForcedColor = value;
    repaint();
}

void CustomDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    // Update shadow color
    QGraphicsDropShadowEffect *effect = qobject_cast<QGraphicsDropShadowEffect *>(graphicsEffect());
    if (effect) {
        effect->setColor(OCC::Utility::getShadowColor(true));
    }

    // Draw round rectangle
    QPainterPath painterPath;
    painterPath.addRoundedRect(rect().marginsRemoved(QMargins(hMargin, vMargin, hMargin, vMargin)),
                               cornerRadius, cornerRadius);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(_backgroundForcedColor == QColor() ? backgroundColor() : _backgroundForcedColor);
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
