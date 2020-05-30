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

#include "fileexclusionnamedialog.h"

#include <QBoxLayout>

namespace KDC {

static const int boxHMargin = 40;
static const int boxHSpacing = 10;
static const int fileNameVMargin = 2;

FileExclusionNameDialog::FileExclusionNameDialog(QWidget *parent)
    : CustomDialog(true, parent)
    , _fileNameLineEdit(nullptr)
    , _validateButton(nullptr)
{
    QVBoxLayout *mainLayout = this->mainLayout();

    // File name
    QHBoxLayout *fileNameHBox = new QHBoxLayout();
    fileNameHBox->setContentsMargins(boxHMargin, fileNameVMargin, boxHMargin, fileNameVMargin);
    fileNameHBox->setSpacing(boxHSpacing);
    mainLayout->addLayout(fileNameHBox);
    mainLayout->addStretch();

    _fileNameLineEdit = new QLineEdit(this);
    fileNameHBox->addWidget(_fileNameLineEdit);

    // Add dialog buttons
    QHBoxLayout *buttonsHBox = new QHBoxLayout();
    buttonsHBox->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    buttonsHBox->setSpacing(boxHSpacing);
    mainLayout->addLayout(buttonsHBox);

    _validateButton = new QPushButton(this);
    _validateButton->setObjectName("defaultbutton");
    _validateButton->setFlat(true);
    _validateButton->setText(tr("VALIDATE"));
    _validateButton->setEnabled(false);
    buttonsHBox->addWidget(_validateButton);

    QPushButton *cancelButton = new QPushButton(this);
    cancelButton->setFlat(true);
    cancelButton->setText(tr("CANCEL"));
    buttonsHBox->addWidget(cancelButton);
    buttonsHBox->addStretch();

    connect(_fileNameLineEdit, &QLineEdit::textEdited, this, &FileExclusionNameDialog::onTextEdited);
    connect(_validateButton, &QPushButton::clicked, this, &FileExclusionNameDialog::onValidateButtonTriggered);
    connect(cancelButton, &QPushButton::clicked, this, &FileExclusionNameDialog::onExit);
    connect(this, &CustomDialog::exit, this, &FileExclusionNameDialog::onExit);
}

QString FileExclusionNameDialog::pattern()
{
    return _fileNameLineEdit->text();
}

void FileExclusionNameDialog::onExit()
{
    reject();
}

void FileExclusionNameDialog::onTextEdited(const QString &text)
{
    _validateButton->setEnabled(!text.isEmpty());
}

void FileExclusionNameDialog::onValidateButtonTriggered(bool checked)
{
    Q_UNUSED(checked)

    accept();
}

}
