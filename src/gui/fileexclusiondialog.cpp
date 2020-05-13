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

#include "fileexclusiondialog.h"

#include <QLabel>

namespace KDC {

FileExclusionDialog::FileExclusionDialog(QWidget *parent)
    : CustomDialog(parent)
{
    QVBoxLayout *mainLayout = this->mainLayout();

    // Title
    QLabel *titleLabel = new QLabel(this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setText(tr("Excluded files"));
    mainLayout->addWidget(titleLabel);

    // Description
    QLabel *descriptionLabel = new QLabel(this);
    descriptionLabel->setObjectName("descriptionLabel");
    descriptionLabel->setText(tr("Add files or folders that will not be synchronized on your computer."));
    mainLayout->addWidget(descriptionLabel);



    mainLayout->addStretch();

    connect(this, &CustomDialog::exit, this, &FileExclusionDialog::onExit);
}

void FileExclusionDialog::onExit()
{
    close();
}

}
