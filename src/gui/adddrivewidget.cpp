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

#include "adddrivewidget.h"
#include "guiutility.h"

#include <QHBoxLayout>
#include <QSizePolicy>
#include <QWidgetAction>

namespace KDC {

static const int boxHMargin= 10;
static const int boxVMargin = 5;
static const int boxSpacing = 10;

AddDriveWidget::AddDriveWidget(QWidget *parent)
    : QPushButton(parent)
    , _addIconSize(QSize())
    , _addIconColor(QColor())
    , _addIconLabel(nullptr)
    , _addTextLabel(nullptr)
{
    setContentsMargins(0, 0, 0, 0);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QHBoxLayout *hbox = new QHBoxLayout(this);
    hbox->setContentsMargins(boxHMargin, boxVMargin, boxHMargin, boxVMargin);
    hbox->setSpacing(boxSpacing);
    setLayout(hbox);

    _addIconLabel = new QLabel(this);
    hbox->addWidget(_addIconLabel);

    _addTextLabel = new QLabel(tr("Add a kDrive"), this);
    _addTextLabel->setObjectName("addTextLabel");
    hbox->addWidget(_addTextLabel);
    hbox->addStretch();

    connect(this, &AddDriveWidget::addIconSizeChanged, this, &AddDriveWidget::onAddIconSizeChanged);
    connect(this, &AddDriveWidget::addIconColorChanged, this, &AddDriveWidget::onAddIconColorChanged);
    connect(this, &AddDriveWidget::clicked, this, &AddDriveWidget::onClick);
}

QSize AddDriveWidget::sizeHint() const
{
    return QSize(_addIconLabel->sizeHint().width()
                 + _addTextLabel->sizeHint().width()
                 + boxSpacing
                 + 2 * boxHMargin,
                 QPushButton::sizeHint().height());
}

void AddDriveWidget::onAddIconSizeChanged()
{
    setAddIcon();
}

void AddDriveWidget::onAddIconColorChanged()
{
    setAddIcon();
}

void AddDriveWidget::onClick(bool checked)
{
    Q_UNUSED(checked)

    emit addDrive();
}

void AddDriveWidget::setAddIcon()
{
    if (_addIconLabel && _addIconSize != QSize() && _addIconColor != QColor()) {
        _addIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/add.svg", _addIconColor)
                                 .pixmap(_addIconSize));
    }
}

}
