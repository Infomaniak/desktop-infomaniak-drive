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

#include "preferencesmenubarwidget.h"
#include "customtoolbutton.h"

namespace KDC {

static const int hMargin = 15;
static const int vMargin = 15;
static const int hButtonsSpacing = 10;

PreferencesMenuBarWidget::PreferencesMenuBarWidget(QWidget *parent)
    : HalfRoundRectWidget(parent)
    , _titleLabel(nullptr)
{
    setContentsMargins(hMargin, 0, hMargin, vMargin);
    setSpacing(0);

    CustomToolButton *backButton = new CustomToolButton(this);
    backButton->setIconPath(":/client/resources/icons/actions/arrow-left.svg");
    backButton->setToolTip(tr("Back to drive list"));
    addWidget(backButton);

    addSpacing(hButtonsSpacing);

    _titleLabel = new QLabel(this);
    _titleLabel->setObjectName("titleLabel");
    _titleLabel->setText(tr("Preferences"));
    addWidget(_titleLabel);

    addStretch();

    connect(backButton, &CustomToolButton::clicked, this, &PreferencesMenuBarWidget::onBackButtonClicked);
}

void PreferencesMenuBarWidget::onBackButtonClicked(bool checked)
{
    Q_UNUSED(checked)

    emit backButtonClicked();
}

}
