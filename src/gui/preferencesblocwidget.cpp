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
#include "guiutility.h"

#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QScrollBar>

namespace KDC {

static const int cornerRadius = 10;
static const int boxHMargin= 15;
static const int boxVMargin = 18;
static const int textVSpacing = 10;
static const int textHSpacing = 10;
static const int shadowBlurRadius = 20;

PreferencesBlocWidget::PreferencesBlocWidget(QWidget *parent)
    : QWidget(parent)
    , _backgroundColor(QColor())
    , _actionIconColor(QColor())
    , _actionIconSize(QSize())
    , _layout(nullptr)
{
    setContentsMargins(0, 0, 0, 0);

    _layout = new QVBoxLayout();
    _layout->setContentsMargins(0, 0, 0, 0);
    _layout->setSpacing(0);
    setLayout(_layout);

    // Shadow
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
    effect->setBlurRadius(shadowBlurRadius);
    effect->setOffset(0);
    setGraphicsEffect(effect);

    connect(this, &PreferencesBlocWidget::actionIconColorChanged, this, &PreferencesBlocWidget::onActionIconColorChanged);
    connect(this, &PreferencesBlocWidget::actionIconSizeChanged, this, &PreferencesBlocWidget::onActionIconSizeChanged);
}

QBoxLayout *PreferencesBlocWidget::addLayout(QBoxLayout::Direction direction)
{
    QBoxLayout *layout = new QBoxLayout(direction);
    layout->setContentsMargins(boxHMargin, boxVMargin, boxHMargin, boxVMargin);
    if (direction == QBoxLayout::Direction::TopToBottom ||
            direction == QBoxLayout::Direction::BottomToTop) {
        layout->setSpacing(textVSpacing);
    }
    else {
        layout->setSpacing(textHSpacing);
    }
    _layout->addLayout(layout);

    return layout;
}

QWidget *PreferencesBlocWidget::addScrollArea(QBoxLayout::Direction direction)
{
    QBoxLayout *hLayout = new QBoxLayout(direction);
    hLayout->setContentsMargins(boxHMargin, boxVMargin, boxHMargin, boxVMargin);
    _layout->addLayout(hLayout);

    QScrollArea *preferencesScrollArea = new QScrollArea(this);
    preferencesScrollArea->setContentsMargins(0, 0, 0, 0);
    preferencesScrollArea->setWidgetResizable(true);
    preferencesScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    hLayout->addWidget(preferencesScrollArea);

    QWidget *widget = new QWidget(this);
    widget->setContentsMargins(0, 0, 0, 0);
    preferencesScrollArea->setWidget(widget);

    QBoxLayout *layout = new QBoxLayout(direction);
    layout->setContentsMargins(0, 0, 0, 0);
    if (direction == QBoxLayout::Direction::TopToBottom ||
            direction == QBoxLayout::Direction::BottomToTop) {
        layout->setSpacing(textVSpacing);
    }
    else {
        layout->setSpacing(textHSpacing);
    }
    widget->setLayout(layout);

    return widget;
}

ClickableWidget *PreferencesBlocWidget::addActionWidget(QVBoxLayout **vLayout)
{
    ClickableWidget *widget = new ClickableWidget(this);
    widget->setContentsMargins(0, 0, 0, 0);
    _layout->addWidget(widget);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(boxHMargin, boxVMargin, boxHMargin, boxVMargin);
    hLayout->setSpacing(0);
    widget->setLayout(hLayout);

    *vLayout = new QVBoxLayout();
    (*vLayout)->setContentsMargins(0, 0, 0, 0);
    (*vLayout)->setSpacing(textVSpacing);
    hLayout->addLayout(*vLayout);
    hLayout->addStretch();

    QLabel *actionIconLabel = new QLabel(this);
    hLayout->addWidget(actionIconLabel);

    return widget;
}

void PreferencesBlocWidget::addSeparator()
{
    QFrame *line = new QFrame(this);
    line->setObjectName("line");
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Plain);
    _layout->addWidget(line);
}

void PreferencesBlocWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    // Shadow
    QGraphicsDropShadowEffect *effect = qobject_cast<QGraphicsDropShadowEffect *>(graphicsEffect());
    if (effect && effect->color() != OCC::Utility::getShadowColor()) {
        effect->setColor(OCC::Utility::getShadowColor());
        effect->update();
    }

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

void PreferencesBlocWidget::setActionIcon()
{
    QList<QLabel *> allActionIconLabels = findChildren<QLabel *>("actionIconLabel");
    for (QLabel *actionIconLabel : allActionIconLabels) {
        actionIconLabel->setPixmap(
                    OCC::Utility::getIconWithColor(":/client/resources/icons/actions/chevron-right.svg", _actionIconColor)
                    .pixmap(_actionIconSize));
    }
}

void PreferencesBlocWidget::onActionIconColorChanged()
{
    setActionIcon();
}

void PreferencesBlocWidget::onActionIconSizeChanged()
{
    setActionIcon();
}

}
