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

#include "preferenceswidget.h"
#include "preferencesblocwidget.h"

#include <QLabel>
#include <QVBoxLayout>

namespace KDC {

static const int boxHMargin= 20;
static const int boxVMargin = 20;
static const int boxSpacing = 12;

PreferencesWidget::PreferencesWidget(QWidget *parent)
    : QWidget(parent)
{
    setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *vbox = new QVBoxLayout();
    vbox->setContentsMargins(boxHMargin, boxVMargin, boxHMargin, boxVMargin);
    vbox->setSpacing(boxSpacing);
    setLayout(vbox);

    QLabel *generalLabel = new QLabel(tr("General"), this);
    generalLabel->setObjectName("generalLabel");
    vbox->addWidget(generalLabel);

    PreferencesBlocWidget *generalBloc = new PreferencesBlocWidget(this);
    vbox->addWidget(generalBloc);

    QLabel *folderConfirmationLabel = new QLabel(tr("Ask for confirmation before synchronizing files greater than"), this);
    generalBloc->layout()->addWidget(folderConfirmationLabel);

    vbox->addStretch();
}

}
