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
#include "customswitch.h"
#include "debuggingdialog.h"
#include "fileexclusiondialog.h"
#include "proxyserverdialog.h"
#include "bandwidthdialog.h"
#include "aboutdialog.h"
#include "custommessagebox.h"
#include "configfile.h"
#include "guiutility.h"
#include "common/utility.h"
#include "logger.h"
#include "theme.h"
#include "version.h"
#include "config.h"
#include "updater/updater.h"
#include "updater/ocupdater.h"
#include "updater/sparkleupdater.h"
#include "folderman.h"

#include <QBoxLayout>
#include <QDesktopServices>
#include <QIntValidator>
#include <QLabel>

namespace KDC {

static const int boxHMargin= 20;
static const int boxVMargin = 20;
static const int boxSpacing = 12;
static const int textHSpacing = 10;
static const int amountLineEditWidth = 85;

static const QString debuggingFolderLink = "debuggingFolderLink";
static const QString versionLink = "versionLink";

Q_LOGGING_CATEGORY(lcPerformancesWidget, "performanceswidget", QtInfoMsg)

PreferencesWidget::PreferencesWidget(QWidget *parent)
    : QWidget(parent)
    , _folderConfirmationAmountLineEdit(nullptr)
    , _debuggingFolderLabel(nullptr)
    , _updateStatusLabel(nullptr)
    , _updateButton(nullptr)
{
    setContentsMargins(0, 0, 0, 0);

    OCC::ConfigFile cfg;

    /*
     *  vBox
     *      generalLabel
     *      generalBloc
     *          folderConfirmationBox
     *              folderConfirmation1HBox
     *                  folderConfirmationLabel
     *                  folderConfirmationSwitch
     *              folderConfirmation2HBox
     *                  _folderConfirmationAmountLineEdit
     *                  folderConfirmationAmountLabel
     *          darkThemeBox
     *              darkThemeLabel
     *              darkThemeSwitch
     *          monochromeIconsBox
     *              monochromeLabel
     *              monochromeSwitch
     *          launchAtStartupBox
     *              launchAtStartupLabel
     *              launchAtStartupSwitch
     *      advancedLabel
     *      advancedBloc
     *          debuggingWidget
     *              debuggingVBox
     *                  debuggingLabel
     *                  _debuggingFolderLabel
     *          filesToExcludeWidget
     *              filesToExcludeVBox
     *                  filesToExcludeLabel
     *          proxyServerWidget
     *              proxyServerVBox
     *                  proxyServerLabel
     *          bandwidthWidget
     *              bandwidthVBox
     *                  bandwidthLabel
     *      versionLabel
     *      versionBloc
     *          versionBox
     *              versionVBox
     *                  versionNumberLabel
     *                  copyrightLabel
     *              _updateButton
     */

    QVBoxLayout *vBox = new QVBoxLayout();
    vBox->setContentsMargins(boxHMargin, boxVMargin, boxHMargin, boxVMargin);
    vBox->setSpacing(boxSpacing);
    setLayout(vBox);

    //
    // General bloc
    //
    QLabel *generalLabel = new QLabel(tr("General"), this);
    generalLabel->setObjectName("blocLabel");
    vBox->addWidget(generalLabel);

    PreferencesBlocWidget *generalBloc = new PreferencesBlocWidget(this);
    vBox->addWidget(generalBloc);

    // Folder synchronization confirmation
    QBoxLayout *folderConfirmationBox = generalBloc->addLayout(QBoxLayout::Direction::TopToBottom);

    QHBoxLayout *folderConfirmation1HBox = new QHBoxLayout();
    folderConfirmation1HBox->setContentsMargins(0, 0, 0, 0);
    folderConfirmation1HBox->setSpacing(0);
    folderConfirmationBox->addLayout(folderConfirmation1HBox);

    QLabel *folderConfirmationLabel = new QLabel(tr("Ask for confirmation before synchronizing folders greater than"), this);
    folderConfirmationLabel->setWordWrap(true);
    folderConfirmation1HBox->addWidget(folderConfirmationLabel);
    folderConfirmation1HBox->setStretchFactor(folderConfirmationLabel, 1);

    CustomSwitch *folderConfirmationSwitch = new CustomSwitch(this);
    folderConfirmationSwitch->setLayoutDirection(Qt::RightToLeft);
    folderConfirmationSwitch->setAttribute(Qt::WA_MacShowFocusRect, false);
    auto folderLimit = cfg.newBigFolderSizeLimit();
    folderConfirmationSwitch->setCheckState(folderLimit.first ? Qt::Checked : Qt::Unchecked);
    folderConfirmation1HBox->addWidget(folderConfirmationSwitch);

    QHBoxLayout *folderConfirmation2HBox = new QHBoxLayout();
    folderConfirmation2HBox->setContentsMargins(0, 0, 0, 0);
    folderConfirmation2HBox->setSpacing(textHSpacing);
    folderConfirmationBox->addLayout(folderConfirmation2HBox);

    _folderConfirmationAmountLineEdit = new QLineEdit(this);
    _folderConfirmationAmountLineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    _folderConfirmationAmountLineEdit->setEnabled(folderLimit.first);
    _folderConfirmationAmountLineEdit->setText(QString::number(folderLimit.second));
    _folderConfirmationAmountLineEdit->setValidator(new QIntValidator(0, 999999, this));
    _folderConfirmationAmountLineEdit->setMinimumWidth(amountLineEditWidth);
    _folderConfirmationAmountLineEdit->setMaximumWidth(amountLineEditWidth);
    folderConfirmation2HBox->addWidget(_folderConfirmationAmountLineEdit);

    QLabel *folderConfirmationAmountLabel = new QLabel(tr("MB"), this);
    folderConfirmationAmountLabel->setObjectName("folderConfirmationAmountLabel");
    folderConfirmation2HBox->addWidget(folderConfirmationAmountLabel);
    folderConfirmation2HBox->addStretch();
    generalBloc->addSeparator();

    // Dark theme activation
    CustomSwitch *darkThemeSwitch = nullptr;
    if (!OCC::Utility::isMac()) {
        QBoxLayout *darkThemeBox = generalBloc->addLayout(QBoxLayout::Direction::LeftToRight);

        QLabel *darkThemeLabel = new QLabel(tr("Activate dark theme"), this);
        darkThemeBox->addWidget(darkThemeLabel);
        darkThemeBox->addStretch();

        darkThemeSwitch = new CustomSwitch(this);
        darkThemeSwitch->setLayoutDirection(Qt::RightToLeft);
        darkThemeSwitch->setAttribute(Qt::WA_MacShowFocusRect, false);
        darkThemeSwitch->setCheckState(cfg.darkTheme() ? Qt::Checked : Qt::Unchecked);
        darkThemeBox->addWidget(darkThemeSwitch);
        generalBloc->addSeparator();
    }

    // Monochrome icons activation
    QBoxLayout *monochromeIconsBox = generalBloc->addLayout(QBoxLayout::Direction::LeftToRight);

    QLabel *monochromeLabel = new QLabel(tr("Activate monochrome icons"), this);
    monochromeIconsBox->addWidget(monochromeLabel);
    monochromeIconsBox->addStretch();

    CustomSwitch *monochromeSwitch = new CustomSwitch(this);
    monochromeSwitch->setLayoutDirection(Qt::RightToLeft);
    monochromeSwitch->setAttribute(Qt::WA_MacShowFocusRect, false);
    monochromeSwitch->setCheckState(cfg.monoIcons() ? Qt::Checked : Qt::Unchecked);
    monochromeIconsBox->addWidget(monochromeSwitch);
    generalBloc->addSeparator();

    // Launch kDrive at startup
    QBoxLayout *launchAtStartupBox = generalBloc->addLayout(QBoxLayout::Direction::LeftToRight);

    QLabel *launchAtStartupLabel = new QLabel(tr("Launch kDrive at startup"), this);
    launchAtStartupBox->addWidget(launchAtStartupLabel);
    launchAtStartupBox->addStretch();

    CustomSwitch *launchAtStartupSwitch = new CustomSwitch(this);
    launchAtStartupSwitch->setLayoutDirection(Qt::RightToLeft);
    launchAtStartupSwitch->setAttribute(Qt::WA_MacShowFocusRect, false);
    bool hasSystemLauchAtStartup = OCC::Utility::hasSystemLaunchOnStartup(OCC::Theme::instance()->appName());
    if (hasSystemLauchAtStartup) {
        // Cannot disable autostart because system-wide autostart is enabled
        launchAtStartupSwitch->setCheckState(Qt::Checked);
        launchAtStartupSwitch->setDisabled(true);
    }
    else {
        bool hasLaunchAtStartup = OCC::Utility::hasLaunchOnStartup(OCC::Theme::instance()->appName());
        launchAtStartupSwitch->setCheckState(hasLaunchAtStartup ? Qt::Checked : Qt::Unchecked);
    }
    launchAtStartupBox->addWidget(launchAtStartupSwitch);
    generalBloc->addSeparator();

    // Drive shortcuts
    QBoxLayout *shortcutsBox = generalBloc->addLayout(QBoxLayout::Direction::LeftToRight);

    QLabel *shortcutsLabel = new QLabel(tr("Add Drive shortcuts in the File Explorer"), this);
    shortcutsBox->addWidget(shortcutsLabel);
    shortcutsBox->addStretch();

    CustomSwitch *shortcutsSwitch = new CustomSwitch(this);
    shortcutsSwitch->setLayoutDirection(Qt::RightToLeft);
    shortcutsSwitch->setAttribute(Qt::WA_MacShowFocusRect, false);
    shortcutsSwitch->setCheckState(cfg.showInExplorerNavigationPane() ? Qt::Checked : Qt::Unchecked);
    shortcutsBox->addWidget(shortcutsSwitch);

    //
    // Advanced bloc
    //
    QLabel *advancedLabel = new QLabel(tr("Advanced"), this);
    advancedLabel->setObjectName("blocLabel");
    vBox->addWidget(advancedLabel);

    PreferencesBlocWidget *advancedBloc = new PreferencesBlocWidget(this);
    vBox->addWidget(advancedBloc);

    // Debugging informations
    QVBoxLayout *debuggingVBox = nullptr;
    ClickableWidget *debuggingWidget = advancedBloc->addActionWidget(&debuggingVBox);

    QLabel *debuggingLabel = new QLabel(tr("Debugging information"), this);
    debuggingVBox->addWidget(debuggingLabel);

    _debuggingFolderLabel = new QLabel(tr("<a style=\"%1\" href=\"%2\">Open debugging folder</a>")
                                       .arg(OCC::Utility::linkStyle)
                                       .arg(debuggingFolderLink),
                                       this);
    _debuggingFolderLabel->setVisible(cfg.automaticLogDir());
    _debuggingFolderLabel->setAttribute(Qt::WA_NoMousePropagation);
    _debuggingFolderLabel->setContextMenuPolicy(Qt::PreventContextMenu);
    debuggingVBox->addWidget(_debuggingFolderLabel);
    advancedBloc->addSeparator();

    // Files to exclude
    QVBoxLayout *filesToExcludeVBox = nullptr;
    ClickableWidget *filesToExcludeWidget = advancedBloc->addActionWidget(&filesToExcludeVBox);

    QLabel *filesToExcludeLabel = new QLabel(tr("Files to exclude"), this);
    filesToExcludeVBox->addWidget(filesToExcludeLabel);
    advancedBloc->addSeparator();

    // Proxy server
    QVBoxLayout *proxyServerVBox = nullptr;
    ClickableWidget *proxyServerWidget = advancedBloc->addActionWidget(&proxyServerVBox);

    QLabel *proxyServerLabel = new QLabel(tr("Proxy server"), this);
    proxyServerVBox->addWidget(proxyServerLabel);
    advancedBloc->addSeparator();

    // Bandwidth
    QVBoxLayout *bandwidthVBox = nullptr;
    ClickableWidget *bandwidthWidget = advancedBloc->addActionWidget(&bandwidthVBox);

    QLabel *bandwidthLabel = new QLabel(tr("Bandwidth"), this);
    bandwidthVBox->addWidget(bandwidthLabel);
    bandwidthVBox->addStretch();

    //
    // Version bloc
    //
    QLabel *versionLabel = new QLabel(tr("Version"), this);
    versionLabel->setObjectName("blocLabel");
    vBox->addWidget(versionLabel);

    PreferencesBlocWidget *versionBloc = new PreferencesBlocWidget(this);
    vBox->addWidget(versionBloc);

    // Version
    QBoxLayout *versionBox = versionBloc->addLayout(QBoxLayout::Direction::LeftToRight);

    QVBoxLayout *versionVBox = new QVBoxLayout();
    versionVBox->setContentsMargins(0, 0, 0, 0);
    versionVBox->setSpacing(0);
    versionBox->addLayout(versionVBox);
    versionBox->setStretchFactor(versionVBox, 1);

    _updateStatusLabel = new QLabel(this);
    _updateStatusLabel->setObjectName("boldTextLabel");
    _updateStatusLabel->setWordWrap(true);
    _updateStatusLabel->setVisible(false);
    versionVBox->addWidget(_updateStatusLabel);

    QLabel *versionNumberLabel = new QLabel(tr("<a style=\"%1\" href=\"%2\">%3</a>")
                                            .arg(OCC::Utility::linkStyle)
                                            .arg(versionLink)
                                            .arg(MIRALL_VERSION_STRING),
                                            this);
    versionNumberLabel->setContextMenuPolicy(Qt::PreventContextMenu);
    versionVBox->addWidget(versionNumberLabel);

    QLabel *copyrightLabel = new QLabel(QString("Copyright %1").arg(APPLICATION_VENDOR), this);
    copyrightLabel->setObjectName("description");
    versionVBox->addWidget(copyrightLabel);

    _updateButton = new QPushButton(this);
    _updateButton->setObjectName("defaultbutton");
    _updateButton->setFlat(true);
    _updateButton->setText(tr("UPDATE"));
    versionBox->addWidget(_updateButton);

    vBox->addStretch();

    connect(folderConfirmationSwitch, &CustomSwitch::clicked, this, &PreferencesWidget::onFolderConfirmationSwitchClicked);
    connect(_folderConfirmationAmountLineEdit, &QLineEdit::textEdited, this, &PreferencesWidget::onFolderConfirmationAmountTextEdited);
    if (darkThemeSwitch) {
        connect(darkThemeSwitch, &CustomSwitch::clicked,this, &PreferencesWidget::onDarkThemeSwitchClicked);
    }
    connect(monochromeSwitch, &CustomSwitch::clicked, this, &PreferencesWidget::onMonochromeSwitchClicked);
    connect(launchAtStartupSwitch, &CustomSwitch::clicked, this, &PreferencesWidget::onLaunchAtStartupSwitchClicked);
    connect(shortcutsSwitch, &CustomSwitch::clicked, this, &PreferencesWidget::onShortcutsSwitchClicked);
    connect(debuggingWidget, &ClickableWidget::clicked, this, &PreferencesWidget::onDebuggingWidgetClicked);
    connect(_debuggingFolderLabel, &QLabel::linkActivated, this, &PreferencesWidget::onLinkActivated);
    connect(filesToExcludeWidget, &ClickableWidget::clicked, this, &PreferencesWidget::onFilesToExcludeWidgetClicked);
    connect(proxyServerWidget, &ClickableWidget::clicked, this, &PreferencesWidget::onProxyServerWidgetClicked);
    connect(bandwidthWidget, &ClickableWidget::clicked, this, &PreferencesWidget::onBandwidthWidgetClicked);
    connect(_updateStatusLabel, &QLabel::linkActivated, this, &PreferencesWidget::onLinkActivated);
    connect(versionNumberLabel, &QLabel::linkActivated, this, &PreferencesWidget::onLinkActivated);
}

void PreferencesWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    onUpdateInfo();
}

void PreferencesWidget::clearUndecidedLists()
{
    for (OCC::Folder *folder : OCC::FolderMan::instance()->map()) {
        // Clear the undecided list
        folder->journalDb()->setSelectiveSyncList(OCC::SyncJournalDb::SelectiveSyncUndecidedList, QStringList());

        // Re-sync folder
        if (folder->isBusy()) {
            folder->slotTerminateSync();
        }

        OCC::FolderMan::instance()->scheduleFolder(folder);

        emit undecidedListsCleared();
    }
}

void PreferencesWidget::updateStatus(QString status, bool updateAvailable)
{
    if (status.isEmpty()) {
        _updateStatusLabel->setVisible(false);
    }
    else {
        _updateStatusLabel->setVisible(true);
        _updateStatusLabel->setText(status);
    }
    _updateButton->setVisible(updateAvailable);
}

void PreferencesWidget::onFolderConfirmationSwitchClicked(bool checked)
{
    OCC::ConfigFile cfg;
    auto folderLimit = cfg.newBigFolderSizeLimit();
    cfg.setNewBigFolderSizeLimit(checked, folderLimit.second);
    _folderConfirmationAmountLineEdit->setEnabled(checked);
    clearUndecidedLists();
}

void PreferencesWidget::onFolderConfirmationAmountTextEdited(const QString &text)
{
    long long lValue = text.toLongLong();
    OCC::ConfigFile cfg;
    auto folderLimit = cfg.newBigFolderSizeLimit();
    cfg.setNewBigFolderSizeLimit(folderLimit.first, lValue);
    clearUndecidedLists();
}

void PreferencesWidget::onDarkThemeSwitchClicked(bool checked)
{
    emit setStyle(checked);
}

void PreferencesWidget::onMonochromeSwitchClicked(bool checked)
{
    OCC::ConfigFile cfg;
    cfg.setMonoIcons(checked);
    OCC::Theme::instance()->setSystrayUseMonoIcons(checked);
}

void PreferencesWidget::onLaunchAtStartupSwitchClicked(bool checked)
{
    OCC::Theme *theme = OCC::Theme::instance();
    OCC::Utility::setLaunchOnStartup(theme->appName(), theme->appNameGUI(), checked);
}

void PreferencesWidget::onShortcutsSwitchClicked(bool checked)
{
    OCC::ConfigFile cfg;
    cfg.setShowInExplorerNavigationPane(checked);
}

void PreferencesWidget::onDebuggingWidgetClicked()
{
    DebuggingDialog *dialog = new DebuggingDialog(this);
    dialog->exec(OCC::Utility::getTopLevelWidget(this)->pos());
    OCC::ConfigFile cfg;
    _debuggingFolderLabel->setVisible(cfg.automaticLogDir());
    repaint();
}

void PreferencesWidget::onFilesToExcludeWidgetClicked()
{
    FileExclusionDialog *dialog = new FileExclusionDialog(this);
    dialog->exec(OCC::Utility::getTopLevelWidget(this)->pos());
}

void PreferencesWidget::onProxyServerWidgetClicked()
{
    ProxyServerDialog *dialog = new ProxyServerDialog(this);
    dialog->exec(OCC::Utility::getTopLevelWidget(this)->pos());
}

void PreferencesWidget::onBandwidthWidgetClicked()
{
    BandwidthDialog *dialog = new BandwidthDialog(this);
    dialog->exec(OCC::Utility::getTopLevelWidget(this)->pos());
}

void PreferencesWidget::onLinkActivated(const QString &link)
{
    if (link == debuggingFolderLink) {
        QString debuggingFolderPath = OCC::Logger::instance()->temporaryFolderLogDirPath();
        QUrl debuggingFolderUrl = OCC::Utility::getUrlFromLocalPath(debuggingFolderPath);
        if (debuggingFolderUrl.isValid()) {
            if (!QDesktopServices::openUrl(debuggingFolderUrl)) {
                qCWarning(lcPerformancesWidget) << "QDesktopServices::openUrl failed for " << debuggingFolderUrl.toString();
                CustomMessageBox *msgBox = new CustomMessageBox(
                            QMessageBox::Warning,
                            tr("Unable to open folder %1.").arg(debuggingFolderUrl.toString()),
                            QMessageBox::Ok, this);
                msgBox->exec();
            }
        }
    }
    else if (link == versionLink) {
        AboutDialog *dialog = new AboutDialog(this);
        dialog->exec(OCC::Utility::getTopLevelWidget(this)->pos());
    }
    else {
        // URL link
        QUrl url = QUrl(link);
        if (url.isValid()) {
            if (!QDesktopServices::openUrl(url)) {
                qCWarning(lcPerformancesWidget) << "QDesktopServices::openUrl failed for " << link;
                CustomMessageBox *msgBox = new CustomMessageBox(
                            QMessageBox::Warning,
                            tr("Unable to open link %1.").arg(link),
                            QMessageBox::Ok, this);
                msgBox->exec();
            }
        }
        else {
            qCWarning(lcPerformancesWidget) << "Invalid link " << link;
            CustomMessageBox *msgBox = new CustomMessageBox(
                        QMessageBox::Warning,
                        tr("Invalid link %1.").arg(link),
                        QMessageBox::Ok, this);
            msgBox->exec();
        }
    }
}

void PreferencesWidget::onUpdateInfo()
{
    OCC::ConfigFile cfg;
    if (cfg.skipUpdateCheck() || !OCC::Updater::instance()) {
        _updateButton->setVisible(false);
        return;
    }

    // Note: the sparkle-updater is not an OCUpdater
    OCC::OCUpdater *ocupdater = qobject_cast<OCC::OCUpdater *>(OCC::Updater::instance());
    if (ocupdater) {
        connect(ocupdater, &OCC::OCUpdater::downloadStateChanged, this, &PreferencesWidget::onUpdateInfo, Qt::UniqueConnection);
        connect(_updateButton, &QPushButton::clicked, ocupdater, &OCC::OCUpdater::slotStartInstaller, Qt::UniqueConnection);
        connect(_updateButton, &QPushButton::clicked, qApp, &QApplication::quit, Qt::UniqueConnection);

        updateStatus(ocupdater->statusString(), ocupdater->downloadState() == OCC::OCUpdater::DownloadComplete);
    }
#if defined(Q_OS_MAC) && defined(HAVE_SPARKLE)
    else if (OCC::SparkleUpdater *sparkleUpdater = qobject_cast<OCC::SparkleUpdater *>(OCC::Updater::instance())) {
        connect(_updateButton, &QPushButton::clicked, sparkleUpdater, &OCC::SparkleUpdater::slotStartInstaller, Qt::UniqueConnection);

        updateStatus(sparkleUpdater->statusString(), sparkleUpdater->updateFound());
    }
#endif
}

}
