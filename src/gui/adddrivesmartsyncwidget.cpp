/*
Infomaniak Drive
Copyright (C) 2021 christophe.larchier@infomaniak.com

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

#include "adddrivesmartsyncwidget.h"
#include "custommessagebox.h"
#include "guiutility.h"
#include "wizard/owncloudwizardcommon.h"

#include <QBoxLayout>
#include <QDesktopServices>
#include <QDir>
#include <QProgressBar>

namespace KDC {

static const int boxHSpacing = 10;
static const int logoBoxVMargin = 20;
static const int progressBarVMargin = 50;
static const int hLogoSpacing = 20;
static const int logoIconSize = 39;
static const QSize logoTextIconSize = QSize(60, 42);
static const int titleBoxVMargin = 20;
static const int pictureBoxVMargin = 40;
static const int textBoxVMargin = 20;
static const int progressBarMin = 0;
static const int progressBarMax = 4;
static const QSize pictureIconSize = QSize(202, 140);
static const QSize checkIconSize = QSize(20, 20);

Q_LOGGING_CATEGORY(lcAddDriveSmartSyncWidget, "gui.AddDriveSmartSyncWidget", QtInfoMsg)

AddDriveSmartSyncWidget::AddDriveSmartSyncWidget(QWidget *parent)
    : QWidget(parent)
    , _logoTextIconLabel(nullptr)
    , _backButton(nullptr)
    , _laterButton(nullptr)
    , _yesButton(nullptr)
    , _logoColor(QColor())
    , _smartSync(false)
{
    initUI();
}

void AddDriveSmartSyncWidget::setButtonIcon(const QColor &value)
{
    if (_backButton) {
        _backButton->setIcon(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/chevron-left.svg", value));
    }
}

void AddDriveSmartSyncWidget::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    setLayout(mainLayout);

    // Logo
    QHBoxLayout *logoHBox = new QHBoxLayout();
    logoHBox->setContentsMargins(0, 0, 0, 0);
    mainLayout->addLayout(logoHBox);
    mainLayout->addSpacing(logoBoxVMargin);

    QLabel *logoIconLabel = new QLabel(this);
    logoIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/logos/kdrive-without-text.svg")
                             .pixmap(QSize(logoIconSize, logoIconSize)));
    logoHBox->addWidget(logoIconLabel);
    logoHBox->addSpacing(hLogoSpacing);

    _logoTextIconLabel = new QLabel(this);
    logoHBox->addWidget(_logoTextIconLabel);
    logoHBox->addStretch();

    // Progress bar
    QProgressBar *progressBar = new QProgressBar(this);
    progressBar->setMinimum(progressBarMin);
    progressBar->setMaximum(progressBarMax);
    progressBar->setValue(1);
    progressBar->setFormat(QString());
    mainLayout->addWidget(progressBar);
    mainLayout->addSpacing(progressBarVMargin);

    // Picture
    QHBoxLayout *pictureHBox = new QHBoxLayout();
    pictureHBox->setContentsMargins(0, 0, 0, 0);
    pictureHBox->setSpacing(boxHSpacing);
    mainLayout->addLayout(pictureHBox);
    mainLayout->addSpacing(pictureBoxVMargin);

    QLabel *pictureIconLabel = new QLabel(this);
    pictureIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/pictures/lite-sync.svg")
                               .pixmap(pictureIconSize));
    pictureIconLabel->setAlignment(Qt::AlignCenter);
    pictureHBox->addWidget(pictureIconLabel);

    // Title
    QLabel *titleLabel = new QLabel(this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setContentsMargins(0, 0, 0, 0);
    titleLabel->setText(tr("Would you like to activate Lite Sync (Beta) ?"));
    titleLabel->setWordWrap(true);
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(titleBoxVMargin);

    // Text
    QLabel *textLabel = new QLabel(this);
    textLabel->setObjectName("largeMediumTextLabel");
    textLabel->setContentsMargins(0, 0, 0, 0);
    textLabel->setText(tr("Lite Sync syncs all your files without using your computer space."
                          " You can browse the files in your kDrive and download them locally whenever you want."
                          " <a style=\"%1\" href=\"%2\">Learn more</a>")
                        .arg(OCC::Utility::linkStyle, OCC::Utility::learnMoreLink));
    textLabel->setWordWrap(true);
    mainLayout->addWidget(textLabel);
    mainLayout->addSpacing(textBoxVMargin);

    // Point 1
    QHBoxLayout *point1HBox = new QHBoxLayout();
    point1HBox->setContentsMargins(0, 0, 0, 0);
    point1HBox->setSpacing(boxHSpacing);
    mainLayout->addLayout(point1HBox);
    mainLayout->addSpacing(textBoxVMargin);

    QLabel *point1IconLabel = new QLabel(this);
    point1IconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/statuts/success.svg")
                               .pixmap(checkIconSize));
    point1HBox->addWidget(point1IconLabel);

    QLabel *point1TextLabel = new QLabel(this);
    point1TextLabel->setObjectName("largeMediumTextLabel");
    point1TextLabel->setText(tr("Conserve your computer space"));
    point1TextLabel->setWordWrap(true);
    point1HBox->addWidget(point1TextLabel);
    point1HBox->setStretchFactor(point1TextLabel, 1);

    // Point 2
    QHBoxLayout *point2HBox = new QHBoxLayout();
    point2HBox->setContentsMargins(0, 0, 0, 0);
    point2HBox->setSpacing(boxHSpacing);
    mainLayout->addLayout(point2HBox);
    mainLayout->addSpacing(textBoxVMargin);
    mainLayout->addStretch();

    QLabel *point2IconLabel = new QLabel(this);
    point2IconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/statuts/success.svg")
                               .pixmap(checkIconSize));
    point2HBox->addWidget(point2IconLabel);

    QLabel *point2TextLabel = new QLabel(this);
    point2TextLabel->setObjectName("largeMediumTextLabel");
    point2TextLabel->setText(tr("Decide which files should be available online or locally"));
    point2TextLabel->setWordWrap(true);
    point2HBox->addWidget(point2TextLabel);
    point2HBox->setStretchFactor(point2TextLabel, 1);

    // Add dialog buttons
    QHBoxLayout *buttonsHBox = new QHBoxLayout();
    buttonsHBox->setContentsMargins(0, 0, 0, 0);
    buttonsHBox->setSpacing(boxHSpacing);
    mainLayout->addLayout(buttonsHBox);

    _backButton = new QPushButton(this);
    _backButton->setObjectName("nondefaultbutton");
    _backButton->setFlat(true);
    buttonsHBox->addWidget(_backButton);
    buttonsHBox->addStretch();

    _laterButton = new QPushButton(this);
    _laterButton->setObjectName("nondefaultbutton");
    _laterButton->setFlat(true);
    _laterButton->setText(tr("LATER"));
    buttonsHBox->addWidget(_laterButton);

    _yesButton = new QPushButton(this);
    _yesButton->setObjectName("defaultbutton");
    _yesButton->setFlat(true);
    _yesButton->setText(tr("YES"));
    buttonsHBox->addWidget(_yesButton);

    connect(textLabel, &QLabel::linkActivated, this, &AddDriveSmartSyncWidget::onLinkActivated);
    connect(_backButton, &QPushButton::clicked, this, &AddDriveSmartSyncWidget::onBackButtonTriggered);
    connect(_laterButton, &QPushButton::clicked, this, &AddDriveSmartSyncWidget::onLaterButtonTriggered);
    connect(_yesButton, &QPushButton::clicked, this, &AddDriveSmartSyncWidget::onYesButtonTriggered);
}

void AddDriveSmartSyncWidget::onLinkActivated(const QString &link)
{
    if (link == OCC::Utility::learnMoreLink) {
        // Learn more: Lite Sync
        if (!QDesktopServices::openUrl(QUrl(LEARNMORE_LITESYNC_URL))) {
            qCWarning(lcAddDriveSmartSyncWidget) << "QDesktopServices::openUrl failed for " << link;
            CustomMessageBox *msgBox = new CustomMessageBox(
                        QMessageBox::Warning,
                        tr("Unable to open link %1.").arg(link),
                        QMessageBox::Ok, this);
            msgBox->exec();
        }
    }
}

void AddDriveSmartSyncWidget::setLogoColor(const QColor &color)
{
    _logoColor = color;
    _logoTextIconLabel->setPixmap(
                OCC::Utility::getIconWithColor(":/client/resources/logos/kdrive-text-only.svg", _logoColor)
                .pixmap(logoTextIconSize));
}

void AddDriveSmartSyncWidget::onBackButtonTriggered(bool checked)
{
    Q_UNUSED(checked)

    emit terminated(false);
}

void AddDriveSmartSyncWidget::onLaterButtonTriggered(bool checked)
{
    Q_UNUSED(checked)

    _smartSync = false;

    emit terminated();
}

void AddDriveSmartSyncWidget::onYesButtonTriggered(bool checked)
{
    Q_UNUSED(checked)

    _smartSync = true;

    emit terminated();
}

}

