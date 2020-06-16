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

#include "aboutdialog.h"
#include "custommessagebox.h"
#include "guiutility.h"
#include "common/utility.h"
#include "common/vfs.h"
#include "version.h"
#include "config.h"

#include <QBoxLayout>
#include <QDesktopServices>
#include <QLabel>
#include <QPushButton>
#include <QSslSocket>

namespace KDC {

static const int boxHMargin = 40;
static const int boxHSpacing = 10;
static const int titleBoxVMargin = 14;
static const int logoBoxVMargin = 10;
static const int hLogoSpacing = 10;
static const int logoIconSize = 39;
static const QSize logoTextIconSize = QSize(60, 42);

static const QString domainLink = "domainLink";
static const QString gitLink = "gitLink";

static const QString githubPrefix = "https://github.com/infomaniak/desktop-infomaniak-drive/commit/";

Q_LOGGING_CATEGORY(lcAboutDialog, "aboutdialog", QtInfoMsg)

AboutDialog::AboutDialog(QWidget *parent)
    : CustomDialog(true, parent)
    , _logoColor(QColor())
    , _logoTextIconLabel(nullptr)
{
    initUI();
}

void AboutDialog::setLogoColor(const QColor &color)
{
    _logoColor = color;
    _logoTextIconLabel->setPixmap(
                OCC::Utility::getIconWithColor(":/client/resources/logos/kdrive-text-only.svg", _logoColor)
                .pixmap(logoTextIconSize));
}

void AboutDialog::initUI()
{
    QVBoxLayout *mainLayout = this->mainLayout();

    // Title
    QLabel *titleLabel = new QLabel(this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    titleLabel->setText(tr("About"));
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(titleBoxVMargin);

    // Logo
    QHBoxLayout *logoHBox = new QHBoxLayout();
    logoHBox->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
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

    // Text
    QLabel *textLabel = new QLabel(this);
    textLabel->setObjectName("largeNormalTextLabel");
    textLabel->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    textLabel->setWordWrap(true);
    textLabel->setText(aboutText());
    mainLayout->addWidget(textLabel);
    mainLayout->addStretch();

    // Add dialog buttons
    QHBoxLayout *buttonsHBox = new QHBoxLayout();
    buttonsHBox->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    buttonsHBox->setSpacing(boxHSpacing);
    mainLayout->addLayout(buttonsHBox);

    QPushButton *closeButton = new QPushButton(this);
    closeButton->setObjectName("defaultbutton");
    closeButton->setFlat(true);
    closeButton->setText(tr("CLOSE"));
    buttonsHBox->addWidget(closeButton);
    buttonsHBox->addStretch();

    connect(closeButton, &QPushButton::clicked, this, &AboutDialog::onExit);
    connect(this, &CustomDialog::exit, this, &AboutDialog::onExit);
    connect(textLabel, &QLabel::linkActivated, this, &AboutDialog::onLinkActivated);
}

QString AboutDialog::aboutText() const
{
    QString about;
    about = tr("Version %1. For more information visit <a style=\"%2\" href=\"%3\">%4</a><br><br>")
            .arg(OCC::Utility::escape(MIRALL_VERSION_STRING))
            .arg(OCC::Utility::linkStyle)
            .arg(domainLink)
            .arg("https://" MIRALL_STRINGIFY(APPLICATION_DOMAIN));
    about += tr("Copyright Infomaniak Network SA, ownCloud GmbH and Nextcloud GmbH<br><br>");
    about += tr("Distributed by %1 and licensed under the GNU Lesser General Public License (LGPL) Version 2.1.<br><br>"
                "%2 and the %2 logo are registered trademarks of %1.<br><br>")
            .arg(OCC::Utility::escape(APPLICATION_VENDOR))
            .arg(OCC::Utility::escape(APPLICATION_NAME));
    about += gitSHA1();
    about += QString("Using virtual files plugin: %1")
            .arg(OCC::Vfs::modeToString(OCC::bestAvailableVfsMode()));

    return about;
}

QString AboutDialog::gitSHA1() const
{
    QString gitText;
#ifdef GIT_SHA1
    const QString gitSha1(QLatin1String(GIT_SHA1));
    gitText = tr("Built from Git revision <a style=\"%1\" href=\"%2\">%3</a> on %4, %5 using Qt %6, %7<br><br>")
            .arg(OCC::Utility::linkStyle)
            .arg(gitLink)
            .arg(gitSha1.left(6))
            .arg(__DATE__)
            .arg(__TIME__)
            .arg(qVersion())
            .arg(QSslSocket::sslLibraryVersionString());
#endif
    return gitText;
}

void AboutDialog::onExit()
{
    accept();
}

void AboutDialog::onLinkActivated(const QString &link)
{
    if (link == domainLink) {
        QUrl domainUrl = QUrl("https://" MIRALL_STRINGIFY(APPLICATION_DOMAIN));
        if (domainUrl.isValid()) {
            if (!QDesktopServices::openUrl(domainUrl)) {
                qCWarning(lcAboutDialog) << "QDesktopServices::openUrl failed for " << domainUrl.toString();
                CustomMessageBox *msgBox = new CustomMessageBox(
                            QMessageBox::Warning,
                            tr("Unable to open debugging folder %1.").arg(domainUrl.toString()),
                            QMessageBox::Ok, this);
                msgBox->exec();
            }
        }
    }
    else if (link == gitLink) {
        const QString gitSha1(QLatin1String(GIT_SHA1));
        QUrl gitUrl = QUrl(githubPrefix + gitSha1);
        if (gitUrl.isValid()) {
            if (!QDesktopServices::openUrl(gitUrl)) {
                qCWarning(lcAboutDialog) << "QDesktopServices::openUrl failed for " << gitUrl.toString();
                CustomMessageBox *msgBox = new CustomMessageBox(
                            QMessageBox::Warning,
                            tr("Unable to open debugging folder %1.").arg(gitUrl.toString()),
                            QMessageBox::Ok, this);
                msgBox->exec();
            }
        }
    }
    accept();
}

}
