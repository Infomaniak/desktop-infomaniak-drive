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

#include "displayerrorswidget.h"
#include "guiutility.h"

#include <QHBoxLayout>
#include <QPainterPath>
#include <QPainter>

namespace KDC {

static const int boxHMargin= 20;
static const int boxVMargin = 5;
static const int boxSpacing = 10;

DisplayErrorsWidget::DisplayErrorsWidget(QWidget *parent)
    : ClickableWidget(parent)
    , _backgroundColor(QColor())
    , _warningIconColor(QColor())
    , _warningIconSize(QSize())
    , _actionIconColor(QColor())
    , _actionIconSize(QSize())
    , _warningIconLabel(nullptr)
    , _actionIconLabel(nullptr)
{
    setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *hbox = new QHBoxLayout(this);
    hbox->setContentsMargins(boxHMargin, boxVMargin, boxHMargin, boxVMargin);
    hbox->setSpacing(boxSpacing);
    setLayout(hbox);

    _warningIconLabel = new QLabel(this);
    hbox->addWidget(_warningIconLabel);

    QLabel *errorTextLabel = new QLabel(tr("Some files couldn't be synchronized"), this);
    errorTextLabel->setObjectName("errorTextLabel");
    hbox->addWidget(errorTextLabel);
    hbox->addStretch();

    _actionIconLabel = new QLabel(this);
    hbox->addWidget(_actionIconLabel);

    connect(this, &DisplayErrorsWidget::warningIconSizeChanged, this, &DisplayErrorsWidget::onWarningIconSizeChanged);
    connect(this, &DisplayErrorsWidget::warningIconColorChanged, this, &DisplayErrorsWidget::onWarningIconColorChanged);
    connect(this, &DisplayErrorsWidget::actionIconColorChanged, this, &DisplayErrorsWidget::onActionIconColorChanged);
    connect(this, &DisplayErrorsWidget::actionIconSizeChanged, this, &DisplayErrorsWidget::onActionIconSizeChanged);
    connect(this, &ClickableWidget::clicked, this, &DisplayErrorsWidget::onClick);
}

void DisplayErrorsWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    // Draw round rectangle
    QPainterPath painterPath;
    painterPath.addRoundedRect(rect(), rect().height() / 2, rect().height() / 2);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(backgroundColor());
    painter.drawPath(painterPath);
}

void DisplayErrorsWidget::setWarningIcon()
{
    if (_warningIconLabel && _warningIconSize != QSize() && _warningIconColor != QColor()) {
        _warningIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/warning.svg", _warningIconColor)
                                     .pixmap(_warningIconSize));
    }
}

void DisplayErrorsWidget::setActionIcon()
{
    if (_actionIconLabel && _actionIconSize != QSize() && _actionIconColor != QColor()) {
        _actionIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/chevron-right.svg", _actionIconColor)
                                    .pixmap(_actionIconSize));
    }
}

void DisplayErrorsWidget::onWarningIconSizeChanged()
{
    setWarningIcon();
}

void DisplayErrorsWidget::onWarningIconColorChanged()
{
    setWarningIcon();
}

void DisplayErrorsWidget::onActionIconColorChanged()
{
    setActionIcon();
}

void DisplayErrorsWidget::onActionIconSizeChanged()
{
    setActionIcon();
}

void DisplayErrorsWidget::onClick()
{
    emit displayErrors();
}

}
