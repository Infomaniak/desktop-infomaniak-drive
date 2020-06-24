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

#include "bigfoldersdialog.h"
#include "guiutility.h"

#include <QBoxLayout>
#include <QDir>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>

namespace KDC {

static const int boxHMargin = 40;
static const int boxHSpacing = 10;
static const int textVMargin = 20;
static const int undecidedListWidgetVMargin = 30;
static const int undecidedListHMargin = 15;
static const int undecidedListVMargin = 15;
static const int undecidedListBoxSpacing = 10;
static const int undecidedItemBoxHSpacing = 10;
static const int undecidedItemBoxVSpacing = 0;
static const int undecidedItemPathSpacing = 6;
static const int undecidedItemPathDriveSpacing = 2;
static const int driveIconSize = 14;
static const int folderNameMaxSize = 50;
static const int locationPathMaxSize = 50;

BigFoldersDialog::BigFoldersDialog(const QStringList &undecidedList, const AccountInfo *accountInfo, QWidget *parent)
    : CustomDialog(true, parent)
{
    QVBoxLayout *mainLayout = this->mainLayout();

    // Text
    QHBoxLayout *textHBox = new QHBoxLayout();
    textHBox->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    mainLayout->addLayout(textHBox);
    mainLayout->addSpacing(textVMargin);

    QLabel *textLabel = new QLabel(this);
    textLabel->setObjectName("largeNormalTextLabel");
    textLabel->setText((tr("Some folders were not synchronized because they are too large :")));
    textLabel->setWordWrap(true);
    textHBox->addWidget(textLabel);

    // Undecided list
    QHBoxLayout *undecidedListWidgetHBox = new QHBoxLayout();
    undecidedListWidgetHBox->setContentsMargins(boxHMargin, 0, boxHMargin, undecidedListWidgetVMargin);
    mainLayout->addLayout(undecidedListWidgetHBox);
    mainLayout->setStretchFactor(undecidedListWidgetHBox, 1);

    QWidget *undecidedListWidget = new QWidget(this);
    undecidedListWidget->setObjectName("undecidedListWidget");
    undecidedListWidgetHBox->addWidget(undecidedListWidget);

    QScrollArea *undecidedListScrollArea = new QScrollArea(this);
    undecidedListScrollArea->setObjectName("undecidedListScrollArea");
    undecidedListScrollArea->setWidget(undecidedListWidget);
    undecidedListScrollArea->setWidgetResizable(true);
    undecidedListWidgetHBox->addWidget(undecidedListScrollArea);

    QVBoxLayout *undecidedListWidgetVBox = new QVBoxLayout();
    undecidedListWidgetVBox->setContentsMargins(undecidedListHMargin, undecidedListVMargin, undecidedListHMargin, undecidedListVMargin);
    undecidedListWidgetVBox->setSpacing(undecidedListBoxSpacing);
    undecidedListWidget->setLayout(undecidedListWidgetVBox);

    for (QString undecidedItem : undecidedList) {
        QDir undecidedItemDir(undecidedItem);
        QString name = undecidedItemDir.dirName();
        QString path = accountInfo->_name + dirSeparator + undecidedItemDir.path();
        path.chop(name.size());
        if (path.endsWith(dirSeparator)) {
            path.chop(1);
        }

        QHBoxLayout *undecidedItemHBox = new QHBoxLayout();
        undecidedItemHBox->setContentsMargins(0, 0, 0, 0);
        undecidedItemHBox->setSpacing(undecidedItemBoxHSpacing);
        undecidedListWidgetVBox->addLayout(undecidedItemHBox);

        QLabel *folderIconLabel = new QLabel(this);
        folderIconLabel->setObjectName("folderIconLabel");
        undecidedItemHBox->addWidget(folderIconLabel);

        QVBoxLayout *undecidedItemVBox = new QVBoxLayout();
        undecidedItemVBox->setContentsMargins(0, 0, 0, 0);
        undecidedItemVBox->setSpacing(undecidedItemBoxVSpacing);
        undecidedItemHBox->addLayout(undecidedItemVBox);
        undecidedItemHBox->setStretchFactor(undecidedItemVBox, 1);

        QLabel *folderName = new QLabel(this);
        folderName->setObjectName("largeNormalBoldTextLabel");
        if (name.size() > folderNameMaxSize) {
            name = name.left(folderNameMaxSize) + "...";
        }
        folderName->setText(name);
        folderName->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
        undecidedItemVBox->addWidget(folderName);

        QHBoxLayout *undecidedItemPathHBox = new QHBoxLayout();
        undecidedItemPathHBox->setContentsMargins(0, 0, 0, 0);
        undecidedItemVBox->addLayout(undecidedItemPathHBox);

        QLabel *locationTextLabel = new QLabel(this);
        locationTextLabel->setObjectName("descriptionLabel");
        locationTextLabel->setText(tr("Location"));
        undecidedItemPathHBox->addWidget(locationTextLabel);
        undecidedItemPathHBox->addSpacing(undecidedItemPathSpacing);

        QLabel *driveIconLabel = new QLabel(this);
        driveIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/drive.svg", accountInfo->_color)
                                  .pixmap(QSize(driveIconSize, driveIconSize)));
        undecidedItemPathHBox->addWidget(driveIconLabel);
        undecidedItemPathHBox->addSpacing(undecidedItemPathDriveSpacing);

        QLabel *locationPathLabel = new QLabel(this);
        locationPathLabel->setObjectName("folderPathLabel");
        if (path.size() > locationPathMaxSize) {
            path = path.left(locationPathMaxSize) + "...";
        }
        locationPathLabel->setText(path);
        locationPathLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
        undecidedItemPathHBox->addWidget(locationPathLabel);
        undecidedItemPathHBox->setStretchFactor(locationPathLabel, 1);
    }

    // Add dialog buttons
    QHBoxLayout *buttonsHBox = new QHBoxLayout();
    buttonsHBox->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    buttonsHBox->setSpacing(boxHSpacing);
    mainLayout->addLayout(buttonsHBox);

    QPushButton *synchronizeButton = new QPushButton(this);
    synchronizeButton->setObjectName("defaultbutton");
    synchronizeButton->setFlat(true);
    synchronizeButton->setText(tr("SYNCHRONIZE ALL"));
    buttonsHBox->addWidget(synchronizeButton);

    QPushButton *doNotSynchronizeButton = new QPushButton(this);
    doNotSynchronizeButton->setObjectName("nondefaultbutton");
    doNotSynchronizeButton->setFlat(true);
    doNotSynchronizeButton->setText(tr("DO NOT SYNCHRONIZE"));
    buttonsHBox->addWidget(doNotSynchronizeButton);
    buttonsHBox->addStretch();

    connect(synchronizeButton, &QPushButton::clicked, this, &BigFoldersDialog::onValidateButtonTriggered);
    connect(doNotSynchronizeButton, &QPushButton::clicked, this, &BigFoldersDialog::onExit);
    connect(this, &CustomDialog::exit, this, &BigFoldersDialog::onExit);
}

void BigFoldersDialog::setFolderIcon()
{
    if (_folderIconSize != QSize() && _folderIconColor != QColor()) {
        QList<QLabel *> labelList = findChildren<QLabel *>("folderIconLabel");
        for (QLabel *label : labelList) {
            label->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/folder.svg", _folderIconColor)
                             .pixmap(_folderIconSize));
        }
    }
}

void BigFoldersDialog::onExit()
{
    reject();
}

void BigFoldersDialog::onValidateButtonTriggered(bool checked)
{
    Q_UNUSED(checked)

    accept();
}

}
