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

#include "errorswidget.h"
#include "guiutility.h"

#include <QHBoxLayout>

namespace KDC {

static const int boxHMargin= 10;
static const int boxVMargin = 5;
static const int boxSpacing = 10;

ErrorsWidget::ErrorsWidget(QWidget *parent)
    : ClickableWidget(parent)
    , _backgroundColor(QColor())
    , _warningIconSize(QSize())
    , _warningIconColor(QColor())
    , _warningIconLabel(nullptr)
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

    connect(this, &ErrorsWidget::warningIconSizeChanged, this, &ErrorsWidget::onWarningIconSizeChanged);
    connect(this, &ErrorsWidget::warningIconColorChanged, this, &ErrorsWidget::onWarningIconColorChanged);
    connect(this, &ClickableWidget::clicked, this, &ErrorsWidget::onClick);
}

void ErrorsWidget::setWarningIcon()
{
    if (_warningIconLabel && _warningIconSize != QSize() && _warningIconColor != QColor()) {
        _warningIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/warning.svg", _warningIconColor)
                                     .pixmap(_warningIconSize));
    }
}

void ErrorsWidget::onWarningIconSizeChanged()
{
    setWarningIcon();
}

void ErrorsWidget::onWarningIconColorChanged()
{
    setWarningIcon();
}

void ErrorsWidget::onClick()
{
    emit displayErrors();
}

}
