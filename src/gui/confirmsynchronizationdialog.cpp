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

#include "confirmsynchronizationdialog.h"
#include "guiutility.h"
#include "common/utility.h"

#include <QBoxLayout>
#include <QLabel>

namespace KDC {

static const int boxHMargin = 40;
static const int boxHSpacing = 10;
static const int titleBoxVMargin = 14;
static const int descriptionBoxVMargin = 85;
static const int summaryBoxHMargin = 110;
static const int iconVSpacing = 20;
static const int nameVSpacing = 5;
static const int arrowVSpacing = 10;
static const int logoSize = 80;
static const int arrowSize = 30;

ConfirmSynchronizationDialog::ConfirmSynchronizationDialog(const QString &localFolderName, qint64 localFolderSize,
                                                           const QString &serverFolderName, qint64 serverFolderSize,
                                                           QWidget *parent)
    : CustomDialog(true, parent)
    , _localFolderName(localFolderName)
    , _localFolderSize(localFolderSize)
    , _serverFolderName(serverFolderName)
    , _serverFolderSize(serverFolderSize)
    , _leftArrowIconLabel(nullptr)
    , _rightArrowIconLabel(nullptr)
    , _backButton(nullptr)
    , _continueButton(nullptr)
{
    initUI();
}

void ConfirmSynchronizationDialog::setButtonIcon(const QColor &value)
{
    if (_backButton) {
        _backButton->setIcon(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/chevron-left.svg", value));
    }
}

void ConfirmSynchronizationDialog::initUI()
{
    QVBoxLayout *mainLayout = this->mainLayout();

    // Title
    QLabel *titleLabel = new QLabel(this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    titleLabel->setText(tr("Summary of your advanced folder synchronization"));
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(titleBoxVMargin);

    // Description
    QLabel *descriptionLabel = new QLabel(this);
    descriptionLabel->setObjectName("descriptionLabel");
    descriptionLabel->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    descriptionLabel->setText(tr("The contents of the folder on your computer will be synchronized to the folder of the selected kDrive and vice versa."));
    descriptionLabel->setWordWrap(true);
    mainLayout->addWidget(descriptionLabel);
    mainLayout->addSpacing(descriptionBoxVMargin);

    // Summary
    QHBoxLayout *summaryLayout = new QHBoxLayout();
    summaryLayout->setContentsMargins(summaryBoxHMargin, 0, summaryBoxHMargin, 0);
    mainLayout->addLayout(summaryLayout);

    QVBoxLayout *summaryLocalLayout = new QVBoxLayout();
    summaryLocalLayout->setContentsMargins(0, 0, 0, 0);
    summaryLayout->addLayout(summaryLocalLayout);

    QLabel *localIconLabel = new QLabel(this);
    localIconLabel->setPixmap(QIcon(":/client/resources/icons/actions/folder-computer.svg").pixmap(QSize(logoSize, logoSize)));
    localIconLabel->setAlignment(Qt::AlignCenter);
    summaryLocalLayout->addWidget(localIconLabel);
    summaryLocalLayout->addSpacing(iconVSpacing);

    QLabel *localNameLabel = new QLabel(this);
    localNameLabel->setObjectName("foldername");
    localNameLabel->setText(_localFolderName);
    localNameLabel->setAlignment(Qt::AlignCenter);
    localNameLabel->setWordWrap(true);
    summaryLocalLayout->addWidget(localNameLabel);
    summaryLocalLayout->addSpacing(nameVSpacing);

    QLabel *localSizeLabel = new QLabel(this);
    localSizeLabel->setObjectName("descriptionLabel");
    localSizeLabel->setText(OCC::Utility::octetsToString(_localFolderSize));
    localSizeLabel->setAlignment(Qt::AlignCenter);
    summaryLocalLayout->addWidget(localSizeLabel);

    QVBoxLayout *summaryArrowsLayout = new QVBoxLayout();
    summaryArrowsLayout->setContentsMargins(0, 0, 0, 0);
    summaryArrowsLayout->addSpacing(arrowVSpacing);
    summaryLayout->addLayout(summaryArrowsLayout);

    _leftArrowIconLabel = new QLabel(this);
    summaryArrowsLayout->addWidget(_leftArrowIconLabel);

    _rightArrowIconLabel = new QLabel(this);
    summaryArrowsLayout->addWidget(_rightArrowIconLabel);
    summaryArrowsLayout->addStretch();

    QVBoxLayout *summaryServerLayout = new QVBoxLayout();
    summaryServerLayout->setContentsMargins(0, 0, 0, 0);
    summaryLayout->addLayout(summaryServerLayout);

    QLabel *serverIconLabel = new QLabel(this);
    serverIconLabel->setPixmap(QIcon(":/client/resources/icons/actions/folder-folder-drive.svg").pixmap(QSize(logoSize, logoSize)));
    serverIconLabel->setAlignment(Qt::AlignCenter);
    summaryServerLayout->addWidget(serverIconLabel);
    summaryServerLayout->addSpacing(iconVSpacing);

    QLabel *serverNameLabel = new QLabel(this);
    serverNameLabel->setObjectName("foldername");
    serverNameLabel->setText(_serverFolderName);
    serverNameLabel->setAlignment(Qt::AlignCenter);
    serverNameLabel->setWordWrap(true);
    summaryServerLayout->addWidget(serverNameLabel);
    summaryServerLayout->addSpacing(nameVSpacing);

    QLabel *serverSizeLabel = new QLabel(this);
    serverSizeLabel->setObjectName("descriptionLabel");
    serverSizeLabel->setText(OCC::Utility::octetsToString(_serverFolderSize));
    serverSizeLabel->setAlignment(Qt::AlignCenter);
    summaryServerLayout->addWidget(serverSizeLabel);

    summaryLayout->setStretchFactor(summaryLocalLayout, 1);
    summaryLayout->setStretchFactor(summaryServerLayout, 1);
    mainLayout->addStretch();

    // Add dialog buttons
    QHBoxLayout *buttonsHBox = new QHBoxLayout();
    buttonsHBox->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    buttonsHBox->setSpacing(boxHSpacing);
    mainLayout->addLayout(buttonsHBox);

    _backButton = new QPushButton(this);
    _backButton->setObjectName("nondefaultbutton");
    _backButton->setFlat(true);
    buttonsHBox->addWidget(_backButton);

    QPushButton *cancelButton = new QPushButton(this);
    cancelButton->setObjectName("nondefaultbutton");
    cancelButton->setFlat(true);
    cancelButton->setText(tr("CANCEL"));
    buttonsHBox->addWidget(cancelButton);
    buttonsHBox->addStretch();

    _continueButton = new QPushButton(this);
    _continueButton->setObjectName("defaultbutton");
    _continueButton->setFlat(true);
    _continueButton->setText(tr("SYNCHRONIZE"));
    buttonsHBox->addWidget(_continueButton);

    connect(_backButton, &QPushButton::clicked, this, &ConfirmSynchronizationDialog::onBackButtonTriggered);
    connect(cancelButton, &QPushButton::clicked, this, &ConfirmSynchronizationDialog::onExit);
    connect(_continueButton, &QPushButton::clicked, this, &ConfirmSynchronizationDialog::onContinueButtonTriggered);
    connect(this, &CustomDialog::exit, this, &ConfirmSynchronizationDialog::onExit);
}

void ConfirmSynchronizationDialog::setArrowIcon()
{
    if (_arrowIconColor != QColor()) {
        _leftArrowIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/arrow-left.svg",
                                                                 _arrowIconColor).pixmap(QSize(arrowSize, arrowSize)));
        _rightArrowIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/arrow-right.svg",
                                                                 _arrowIconColor).pixmap(QSize(arrowSize, arrowSize)));
    }
}

void ConfirmSynchronizationDialog::onExit()
{
    reject();
}

void ConfirmSynchronizationDialog::onBackButtonTriggered(bool checked)
{
    Q_UNUSED(checked)

    done(-1);
}

void ConfirmSynchronizationDialog::onContinueButtonTriggered(bool checked)
{
    Q_UNUSED(checked)

    accept();
}

}
