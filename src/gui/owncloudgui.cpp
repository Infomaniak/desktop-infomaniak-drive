/*
 * Copyright (C) by Klaas Freitag <freitag@owncloud.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#include "application.h"
#include "owncloudgui.h"
#include "theme.h"
#include "folderman.h"
#include "configfile.h"
#include "progressdispatcher.h"
#include "owncloudsetupwizard.h"
#include "sharedialog.h"
#include "settingsdialog.h"
#include "logger.h"
#include "logbrowser.h"
#include "account.h"
#include "accountstate.h"
#include "openfilemanager.h"
#include "accountmanager.h"
#include "common/syncjournalfilerecord.h"
#include "creds/abstractcredentials.h"
#include "guiutility.h"

#include <QDesktopServices>
#include <QDir>
#include <QMessageBox>
#include <QDialog>
#include <QHBoxLayout>

#if defined(Q_OS_X11)
#include <QX11Info>
#endif

#ifdef WITH_LIBCLOUDPROVIDERS
#include "libcloudproviders/libcloudproviders.h"
#endif

namespace OCC {

const char propertyAccountC[] = "oc_account";

ownCloudGui::ownCloudGui(Application *parent)
    : QObject(parent)
    , _tray(nullptr)
    , _logBrowser(nullptr)
#ifdef KDRIVE_V2_NEW_SETTINGS
    , _parametersDialog(new KDC::ParametersDialog())
#else
    , _settingsDialog(new SettingsDialog(this))
#endif
    , _recentActionsMenu(nullptr)
    , _notificationEnableDate(QDateTime())
    , _app(parent)
{
    _tray = new Systray();
    _tray->setParent(this);

    // for the beginning, set the offline icon until the account was verified
    _tray->setIcon(Theme::instance()->folderOfflineIcon(/*systray?*/ true, /*currently visible?*/ false));

    connect(_tray.data(), &QSystemTrayIcon::activated, this, &ownCloudGui::slotTrayClicked);

#ifdef KDRIVE_V2
    _tray->show();
    setupPopover();
#else
    setupActions();
    setupPopover();
    _tray->show();
#endif

#ifdef WITH_LIBCLOUDPROVIDERS
    auto exporter = new LibCloudProviders(this);
    exporter->start();
    connect(exporter, &LibCloudProviders::showSettings, this, &ownCloudGui::slotShowSettings);
#endif

    ProgressDispatcher *pd = ProgressDispatcher::instance();

    connect(pd, &ProgressDispatcher::progressInfo, this, &ownCloudGui::slotUpdateProgress);
    connect(pd, &ProgressDispatcher::itemCompleted, this, &ownCloudGui::slotItemCompleted);

    FolderMan *folderMan = FolderMan::instance();
    connect(folderMan, &FolderMan::folderSyncStateChange, this, &ownCloudGui::slotSyncStateChange);

    connect(AccountManager::instance(), &AccountManager::accountAdded, this, &ownCloudGui::onRefreshAccountList);
    connect(AccountManager::instance(), &AccountManager::accountRemoved, this, &ownCloudGui::onRefreshAccountList);

    connect(Logger::instance(), &Logger::guiLog, this, &ownCloudGui::slotShowTrayMessage);
    connect(Logger::instance(), &Logger::optionalGuiLog, this, &ownCloudGui::slotShowOptionalTrayMessage);
    connect(Logger::instance(), &Logger::guiMessage, this, &ownCloudGui::slotShowGuiMessage);
}

// This should rather be in application.... or rather in ConfigFile?
void ownCloudGui::slotOpenParametersDialog()
{
    // if account is set up, start the configuration wizard.
    if (!AccountManager::instance()->accounts().isEmpty()) {
#ifdef KDRIVE_V2_NEW_SETTINGS
        if (_parametersDialog.isNull() || QApplication::activeWindow() != _parametersDialog) {
            slotShowParametersDialog();
        } else {
            _parametersDialog->close();
        }
#else
        if (_settingsDialog.isNull() || QApplication::activeWindow() != _settingsDialog) {
            slotShowParametersDialog();
        } else {
            _settingsDialog->close();
        }
#endif
    } else {
        qCInfo(lcApplication) << "No configured folders yet, starting setup wizard";
        slotNewAccountWizard();
    }
}

void ownCloudGui::slotTrayClicked(QSystemTrayIcon::ActivationReason reason)
{
    if (_workaroundFakeDoubleClick) {
        static QElapsedTimer last_click;
        if (last_click.isValid() && last_click.elapsed() < 200) {
            return;
        }
        last_click.start();
    }

    // Left click
    if (reason == QSystemTrayIcon::Trigger) {
        if (OwncloudSetupWizard::bringWizardToFrontIfVisible()) {
            // brought wizard to front
        } else if (_shareDialogs.size() > 0) {
            // Share dialog(s) be hidden by other apps, bring them back
            Q_FOREACH (const QPointer<ShareDialog> &shareDialog, _shareDialogs) {
                Q_ASSERT(shareDialog.data());
                raiseDialog(shareDialog);
            }
        } else {
#ifdef KDRIVE_V2
            if (_synthesisPopover) {
                _synthesisPopover->setPosition(_tray->geometry());
                raiseDialog(_synthesisPopover.get());
            }
#else
#ifdef Q_OS_MAC
            // on macOS, a left click always opens menu.
            // However if the settings dialog is already visible but hidden
            // by other applications, this will bring it to the front.
            if (!_settingsDialog.isNull() && _settingsDialog->isVisible()) {
                raiseDialog(_settingsDialog.data());
            }
#else
            slotOpenParametersDialog();
#endif
#endif
        }
    }
    // FIXME: Also make sure that any auto updater dialogue https://github.com/owncloud/client/issues/5613
    // or SSL error dialog also comes to front.
}

void ownCloudGui::slotSyncStateChange(Folder *folder)
{
    slotComputeOverallSyncStatus();
    updatePopoverNeeded();

    if (!folder) {
        return; // Valid, just a general GUI redraw was needed.
    }

    auto result = folder->syncResult();

    qCInfo(lcApplication) << "Sync state changed for folder " << folder->remoteUrl().toString() << ": " << result.statusString();

    if (result.status() == SyncResult::Success
        || result.status() == SyncResult::Problem
        || result.status() == SyncResult::SyncAbortRequested
        || result.status() == SyncResult::Error) {
        Logger::instance()->enterNextLogFile();
    }

    if (result.status() == SyncResult::NotYetStarted) {
#ifndef KDRIVE_V2_NEW_SETTINGS
        _settingsDialog->slotRefreshActivity(folder->accountState());
#endif
    }
}

void ownCloudGui::slotFoldersChanged()
{
    slotComputeOverallSyncStatus();
    updatePopoverNeeded();
}

void ownCloudGui::slotOpenPath(const QString &path)
{
    showInFileManager(path);
}

void ownCloudGui::slotAccountStateChanged()
{
    updatePopoverNeeded();
    slotComputeOverallSyncStatus();
}

void ownCloudGui::slotTrayMessageIfServerUnsupported(Account *account)
{
    if (account->serverVersionUnsupported()) {
        slotShowTrayMessage(
            tr("Unsupported Server Version"),
            tr("The server on account %1 runs an unsupported version %2. "
               "Using this client with unsupported server versions is untested and "
               "potentially dangerous. Proceed at your own risk.")
                .arg(account->displayName(), account->serverVersion()));
    }
}

void ownCloudGui::slotShowErrors()
{
    slotShowParametersDialog();
#ifndef KDRIVE_V2
    _settingsDialog->showIssuesList("");
#endif
}

void ownCloudGui::slotComputeOverallSyncStatus()
{
    bool allSignedOut = true;
    bool allPaused = true;
    bool allDisconnected = true;
    QVector<AccountStatePtr> problemAccounts;
#ifndef KDRIVE_V2
    auto setStatusText = [&](const QString &text) {
        // Don't overwrite the status if we're currently syncing
        if (FolderMan::instance()->isAnySyncRunning())
            return;
        _actionStatus->setText(text);
    };
#endif

    foreach (auto a, AccountManager::instance()->accounts()) {
        if (!a->isSignedOut()) {
            allSignedOut = false;
        }
        if (!a->isConnected()) {
            problemAccounts.append(a);
        } else {
            allDisconnected = false;
        }
    }
    foreach (Folder *f, FolderMan::instance()->map()) {
        if (!f->syncPaused()) {
            allPaused = false;
        }
    }

    if (!problemAccounts.empty()) {
#ifdef KDRIVE_V2
        _tray->setIcon(Theme::instance()->folderOfflineIcon(true, true));
#else
        _tray->setIcon(Theme::instance()->folderOfflineIcon(true, popoverVisible()));
        if (allDisconnected) {
            setStatusText(tr("Disconnected"));
        } else {
            setStatusText(tr("Disconnected from some accounts"));
        }
#endif
#ifdef Q_OS_WIN
        // Windows has a 128-char tray tooltip length limit.
        QStringList accountNames;
        foreach (AccountStatePtr a, problemAccounts) {
            accountNames.append(a->account()->displayName());
        }
        _tray->setToolTip(tr("Disconnected from %1").arg(accountNames.join(QLatin1String(", "))));
#else
        QStringList messages;
        messages.append(tr("Disconnected from accounts:"));
        foreach (AccountStatePtr a, problemAccounts) {
            QString message = tr("Account %1: %2").arg(a->account()->displayName(), a->stateString(a->state()));
            if (!a->connectionErrors().empty()) {
                message += QLatin1String("\n");
                message += a->connectionErrors().join(QLatin1String("\n"));
            }
            messages.append(message);
        }
        _tray->setToolTip(messages.join(QLatin1String("\n\n")));
#endif
        return;
    }

    if (allSignedOut) {
#ifdef KDRIVE_V2
        _tray->setIcon(Theme::instance()->folderOfflineIcon(true, true));
#else
        _tray->setIcon(Theme::instance()->folderOfflineIcon(true, popoverVisible()));
#endif
        _tray->setToolTip(tr("Please sign in"));
#ifndef KDRIVE_V2
        setStatusText(tr("Signed out"));
#endif
        return;
    } else if (allPaused) {
#ifdef KDRIVE_V2
        _tray->setIcon(Theme::instance()->syncStateIcon(SyncResult::Paused, true, true));
#else
        _tray->setIcon(Theme::instance()->syncStateIcon(SyncResult::Paused, true, popoverVisible()));
#endif
        _tray->setToolTip(tr("Account synchronization is disabled"));
#ifndef KDRIVE_V2
        setStatusText(tr("Synchronization is paused"));
#endif
        return;
    }

    // display the info of the least successful sync (eg. do not just display the result of the latest sync)
    QString trayMessage;
    FolderMan *folderMan = FolderMan::instance();
    Folder::Map map = folderMan->map();

    SyncResult::Status overallStatus = SyncResult::Undefined;
    bool hasUnresolvedConflicts = false;
    FolderMan::trayOverallStatus(map.values(), &overallStatus, &hasUnresolvedConflicts);

    // If the sync succeeded but there are unresolved conflicts,
    // show the problem icon!
    auto iconStatus = overallStatus;
    if (iconStatus == SyncResult::Success && hasUnresolvedConflicts) {
        iconStatus = SyncResult::Problem;
    }

    // If we don't get a status for whatever reason, that's a Problem
    if (iconStatus == SyncResult::Undefined) {
        iconStatus = SyncResult::Problem;
    }

    // Set sytray icon
    Application *app = static_cast<Application *>(qApp);
#ifdef KDRIVE_V2
    QIcon statusIcon = Theme::instance()->syncStateIcon(iconStatus, true, true, app->getAlert());
#else
    QIcon statusIcon = Theme::instance()->syncStateIcon(iconStatus, true, popoverVisible(), app->getAlert());
#endif
    _tray->setIcon(statusIcon);

#ifndef KDRIVE_V2
    _actionShowErrors->setEnabled(_settingsDialog ? _settingsDialog->getErrorCount() > 0 : false);
#endif

    // create the tray blob message, check if we have an defined state
    if (map.count() > 0) {
#ifdef Q_OS_WIN
        // Windows has a 128-char tray tooltip length limit.
        trayMessage = folderMan->trayTooltipStatusString(overallStatus, hasUnresolvedConflicts, false);
#else
        QStringList allStatusStrings;
        foreach (Folder *folder, map.values()) {
            QString folderMessage = FolderMan::trayTooltipStatusString(
                folder->syncResult().status(),
                folder->syncResult().hasUnresolvedConflicts(),
                folder->syncPaused());
            allStatusStrings += tr("Folder %1: %2").arg(folder->shortGuiLocalPath(), folderMessage);
        }
        trayMessage = allStatusStrings.join(QLatin1String("\n"));
#endif
        _tray->setToolTip(trayMessage);

#ifndef KDRIVE_V2
        if (overallStatus == SyncResult::Success || overallStatus == SyncResult::Problem) {
            if (hasUnresolvedConflicts) {
                setStatusText(tr("Unresolved conflicts"));
            } else {
                setStatusText(tr("Up to date"));
            }
        } else if (overallStatus == SyncResult::Paused) {
            setStatusText(tr("Synchronization is paused"));
        } else {
            setStatusText(tr("Error during synchronization"));
        }
#endif
    } else {
        _tray->setToolTip(tr("There are no sync folders configured."));
#ifndef KDRIVE_V2
        setStatusText(tr("No sync folders configured"));
#endif
    }
}

#ifndef KDRIVE_V2
void ownCloudGui::addAccountContextMenu(AccountStatePtr accountState, QMenu *menu, bool separateMenu)
{
    // Only show the name in the action if it's not part of an
    // account sub menu.
    QString browserOpen = tr("Open in browser");
    if (!separateMenu) {
        browserOpen = tr("Open %1 in browser").arg(Theme::instance()->appNameGUI());
    }
    auto actionOpenoC = menu->addAction(browserOpen);
    actionOpenoC->setProperty(propertyAccountC, QVariant::fromValue(accountState->account()));
    QObject::connect(actionOpenoC, &QAction::triggered, this, &ownCloudGui::slotOpenWebview);

    FolderMan *folderMan = FolderMan::instance();
    bool firstFolder = true;
    bool singleSyncFolder = folderMan->map().size() == 1 && Theme::instance()->singleSyncFolder();
    bool onePaused = false;
    bool allPaused = true;
    foreach (Folder *folder, folderMan->map()) {
        if (folder->accountState() != accountState.data()) {
            continue;
        }

        if (folder->syncPaused()) {
            onePaused = true;
        } else {
            allPaused = false;
        }

        if (firstFolder && !singleSyncFolder) {
            firstFolder = false;
            menu->addSeparator();
            menu->addAction(tr("Managed Folders:"))->setDisabled(true);
        }

        QAction *action = menu->addAction(tr("Open folder '%1'").arg(folder->shortGuiLocalPath()));
        auto alias = folder->alias();
        connect(action, &QAction::triggered, this, [this, alias] { this->slotFolderOpenAction(alias); });
    }

    menu->addSeparator();
    if (separateMenu) {
        if (onePaused) {
            QAction *enable = menu->addAction(tr("Unpause all folders"));
            enable->setProperty(propertyAccountC, QVariant::fromValue(accountState));
            connect(enable, &QAction::triggered, this, &ownCloudGui::slotUnpauseAllFolders);
        }
        if (!allPaused) {
            QAction *enable = menu->addAction(tr("Pause all folders"));
            enable->setProperty(propertyAccountC, QVariant::fromValue(accountState));
            connect(enable, &QAction::triggered, this, &ownCloudGui::slotPauseAllFolders);
        }

        if (accountState->isSignedOut()) {
            QAction *signin = menu->addAction(tr("Log in..."));
            signin->setProperty(propertyAccountC, QVariant::fromValue(accountState));
            connect(signin, &QAction::triggered, this, &ownCloudGui::slotLogin);
        } else {
            QAction *signout = menu->addAction(tr("Log out"));
            signout->setProperty(propertyAccountC, QVariant::fromValue(accountState));
            connect(signout, &QAction::triggered, this, &ownCloudGui::slotLogout);
        }
    }
}
#endif

#ifndef KDRIVE_V2
void ownCloudGui::slotPopoverAboutToShow()
{
    _popoverVisibleManual = true;

    // Update icon in sys tray, as it might change depending on the context menu state
    slotComputeOverallSyncStatus();

    if (!_workaroundNoAboutToShowUpdate) {
        updatePopover();
    }
}

void ownCloudGui::slotPopoverAboutToHide()
{
    _popoverVisibleManual = false;

    // Update icon in sys tray, as it might change depending on the context menu state
    slotComputeOverallSyncStatus();
}
#endif

#ifndef KDRIVE_V2
bool ownCloudGui::popoverVisible() const
{
    // On some platforms isVisible doesn't work and always returns false,
    // elsewhere aboutToHide is unreliable.
    if (_workaroundManualVisibility) {
        return _popoverVisibleManual;
    }

    return _contextMenu && _contextMenu->isVisible();
}
#endif

void ownCloudGui::hideAndShowTray()
{
    _tray->hide();
    _tray->show();
}

#ifndef KDRIVE_V2
static bool minimalTrayMenu()
{
    static QByteArray var = qgetenv("OWNCLOUD_MINIMAL_TRAY_MENU");
    return !var.isEmpty();
}
#endif

#ifndef KDRIVE_V2
static bool updateWhileVisible()
{
    static QByteArray var = qgetenv("OWNCLOUD_TRAY_UPDATE_WHILE_VISIBLE");
    if (var == "1") {
        return true;
    } else if (var == "0") {
        return false;
    } else {
        // triggers bug on OS X: https://bugreports.qt.io/browse/QTBUG-54845
        // or flickering on Xubuntu
        return false;
    }
}
#endif

static QByteArray envForceQDBusTrayWorkaround()
{
    static QByteArray var = qgetenv("OWNCLOUD_FORCE_QDBUS_TRAY_WORKAROUND");
    return var;
}

static QByteArray envForceWorkaroundShowAndHideTray()
{
    static QByteArray var = qgetenv("OWNCLOUD_FORCE_TRAY_SHOW_HIDE");
    return var;
}

static QByteArray envForceWorkaroundNoAboutToShowUpdate()
{
    static QByteArray var = qgetenv("OWNCLOUD_FORCE_TRAY_NO_ABOUT_TO_SHOW");
    return var;
}

static QByteArray envForceWorkaroundFakeDoubleClick()
{
    static QByteArray var = qgetenv("OWNCLOUD_FORCE_TRAY_FAKE_DOUBLE_CLICK");
    return var;
}

static QByteArray envForceWorkaroundManualVisibility()
{
    static QByteArray var = qgetenv("OWNCLOUD_FORCE_TRAY_MANUAL_VISIBILITY");
    return var;
}

void ownCloudGui::setupPopover()
{
#ifdef KDRIVE_V2
    if (_synthesisPopover) {
        return;
    }

    _synthesisPopover.reset(new KDC::SynthesisPopover(_app->debugMode()));
    connect(_synthesisPopover.get(), &KDC::SynthesisPopover::openParametersDialog, this, &ownCloudGui::slotShowParametersDialog);
    connect(_synthesisPopover.get(), &KDC::SynthesisPopover::openShareDialogPublicLinks, this, &ownCloudGui::slotShowShareDialogPublicLinks);
    connect(_synthesisPopover.get(), &KDC::SynthesisPopover::exit, _app, &Application::quit);
    connect(_synthesisPopover.get(), &KDC::SynthesisPopover::addDrive, this, &ownCloudGui::slotNewAccountWizard);
    connect(_synthesisPopover.get(), &KDC::SynthesisPopover::disableNotifications, this, &ownCloudGui::slotDisableNotifications);
    connect(_synthesisPopover.get(), &KDC::SynthesisPopover::applyStyle, this, &ownCloudGui::slotApplyStyle);
    connect(_synthesisPopover.get(), &KDC::SynthesisPopover::crash, _app, &Application::slotCrash);
    connect(_synthesisPopover.get(), &KDC::SynthesisPopover::crashEnforce, _app, &Application::slotCrashEnforce);
    connect(_synthesisPopover.get(), &KDC::SynthesisPopover::crashFatal, _app, &Application::slotCrashFatal);
#else
    if (_contextMenu) {
        return;
    }

    _contextMenu.reset(new QMenu());
    _contextMenu->setTitle(Theme::instance()->appNameGUI());

    _recentActionsMenu = new QMenu(tr("Recent Changes"), _contextMenu.data());

    // this must be called only once after creating the context menu, or
    // it will trigger a bug in Ubuntu's SNI bridge patch (11.10, 12.04).
    _tray->setContextMenu(_contextMenu.data());

    // The tray menu is surprisingly problematic. Being able to switch to
    // a minimal version of it is a useful workaround and testing tool.
    if (minimalTrayMenu()) {
        if (! Theme::instance()->about().isEmpty()) {
            _contextMenu->addSeparator();
            _contextMenu->addAction(_actionAbout);
        }
        _contextMenu->addAction(_actionQuit);
        return;
    }
#endif

    auto applyEnvVariable = [](bool *sw, const QByteArray &value) {
        if (value == "1")
            *sw = true;
        if (value == "0")
            *sw = false;
    };

    // This is an old compound flag that people might still depend on
    bool qdbusmenuWorkarounds = false;
    applyEnvVariable(&qdbusmenuWorkarounds, envForceQDBusTrayWorkaround());
    if (qdbusmenuWorkarounds) {
        _workaroundFakeDoubleClick = true;
        _workaroundNoAboutToShowUpdate = true;
        _workaroundShowAndHideTray = true;
    }

#ifdef Q_OS_MAC
    // https://bugreports.qt.io/browse/QTBUG-54633
    _workaroundNoAboutToShowUpdate = true;
    _workaroundManualVisibility = true;
#endif

#ifdef Q_OS_LINUX
#ifndef KDRIVE_V2
    // For KDE sessions if the platform plugin is missing,
    // neither aboutToShow() updates nor the isVisible() call
    // work. At least aboutToHide is reliable.
    // https://github.com/owncloud/client/issues/6545
    static QByteArray xdgCurrentDesktop = qgetenv("XDG_CURRENT_DESKTOP");
    static QByteArray desktopSession = qgetenv("DESKTOP_SESSION");
    bool isKde =
        xdgCurrentDesktop.contains("KDE")
        || desktopSession.contains("plasma")
        || desktopSession.contains("kde");
    QObject *platformMenu = reinterpret_cast<QObject *>(_tray->contextMenu()->platformMenu());
    if (isKde && platformMenu && platformMenu->metaObject()->className() == QLatin1String("QDBusPlatformMenu")) {
        _workaroundManualVisibility = true;
        _workaroundNoAboutToShowUpdate = true;
    }
#endif
#endif

    applyEnvVariable(&_workaroundNoAboutToShowUpdate, envForceWorkaroundNoAboutToShowUpdate());
    applyEnvVariable(&_workaroundFakeDoubleClick, envForceWorkaroundFakeDoubleClick());
    applyEnvVariable(&_workaroundShowAndHideTray, envForceWorkaroundShowAndHideTray());
    applyEnvVariable(&_workaroundManualVisibility, envForceWorkaroundManualVisibility());

    qCInfo(lcApplication) << "Tray menu workarounds:"
                          << "noabouttoshow:" << _workaroundNoAboutToShowUpdate
                          << "fakedoubleclick:" << _workaroundFakeDoubleClick
                          << "showhide:" << _workaroundShowAndHideTray
                          << "manualvisibility:" << _workaroundManualVisibility;


    connect(&_delayedTrayUpdateTimer, &QTimer::timeout, this, &ownCloudGui::updatePopover);
    _delayedTrayUpdateTimer.setInterval(2 * 1000);
    _delayedTrayUpdateTimer.setSingleShot(true);

#ifndef KDRIVE_V2
    connect(_contextMenu.data(), SIGNAL(aboutToShow()), SLOT(slotPopoverAboutToShow()));
    // unfortunately aboutToHide is unreliable, it seems to work on OSX though
    connect(_contextMenu.data(), SIGNAL(aboutToHide()), SLOT(slotPopoverAboutToHide()));
#endif

    // Populate the context menu now.
    updatePopover();
}

void ownCloudGui::updatePopover()
{
#ifndef KDRIVE_V2
    if (minimalTrayMenu()) {
        return;
    }
#endif

    // If it's visible, we can't update live, and it won't be updated lazily: reschedule
#ifndef KDRIVE_V2
    if (popoverVisible() && !updateWhileVisible() && _workaroundNoAboutToShowUpdate) {
        if (!_delayedTrayUpdateTimer.isActive()) {
            _delayedTrayUpdateTimer.start();
        }
        return;
    }
#endif

    if (_workaroundShowAndHideTray) {
        // To make tray menu updates work with these bugs (see setupPopover)
        // we need to hide and show the tray icon. We don't want to do that
        // while it's visible!
#ifndef KDRIVE_V2
        if (popoverVisible()) {
#endif
            if (!_delayedTrayUpdateTimer.isActive()) {
                _delayedTrayUpdateTimer.start();
            }
#ifndef KDRIVE_V2
            return;
        }
#endif
        _tray->hide();
    }

#ifndef KDRIVE_V2
    _contextMenu->clear();
    slotRebuildRecentMenus();

    // We must call deleteLater because we might be called from the press in one of the actions.
    foreach (auto menu, _accountMenus) {
        menu->deleteLater();
    }
    _accountMenus.clear();

    auto accountList = AccountManager::instance()->accounts();

    bool isConfigured = (!accountList.isEmpty());
    bool atLeastOneConnected = false;
    bool atLeastOneSignedOut = false;
    bool atLeastOneSignedIn = false;
    bool atLeastOnePaused = false;
    bool atLeastOneNotPaused = false;
    foreach (auto a, accountList) {
        if (a->isConnected()) {
            atLeastOneConnected = true;
        }
        if (a->isSignedOut()) {
            atLeastOneSignedOut = true;
        } else {
            atLeastOneSignedIn = true;
        }
    }
    foreach (auto f, FolderMan::instance()->map()) {
        if (f->syncPaused()) {
            atLeastOnePaused = true;
        } else {
            atLeastOneNotPaused = true;
        }
    }

    if (accountList.count() > 1) {
        foreach (AccountStatePtr account, accountList) {
            QMenu *accountMenu = new QMenu(account->account()->driveName(), _contextMenu.data());
            _accountMenus.append(accountMenu);
            _contextMenu->addMenu(accountMenu);

            addAccountContextMenu(account, accountMenu, true);
        }
    } else if (accountList.count() == 1) {
        addAccountContextMenu(accountList.first(), _contextMenu.data(), false);
    }

    _contextMenu->addSeparator();

    _contextMenu->addAction(_actionShowErrors);

    _contextMenu->addSeparator();

    _contextMenu->addAction(_actionStatus);
    if (isConfigured && atLeastOneConnected) {
        _contextMenu->addMenu(_recentActionsMenu);
    }

    _contextMenu->addSeparator();

    if (accountList.isEmpty()) {
        _contextMenu->addAction(_actionNewAccountWizard);
    }
    _contextMenu->addAction(_actionSettings);
    if (!Theme::instance()->helpUrl().isEmpty()) {
        _contextMenu->addAction(_actionHelp);
    }

    if (_actionCrash) {
        _contextMenu->addAction(_actionCrash);
        _contextMenu->addAction(_actionCrashEnforce);
        _contextMenu->addAction(_actionCrashFatal);
    }

    _contextMenu->addSeparator();
    if (atLeastOnePaused) {
        QString text;
        if (accountList.count() > 1) {
            text = tr("Unpause all synchronization");
        } else {
            text = tr("Unpause synchronization");
        }
        QAction *action = _contextMenu->addAction(text);
        connect(action, &QAction::triggered, this, &ownCloudGui::slotUnpauseAllFolders);
    }
    if (atLeastOneNotPaused) {
        QString text;
        if (accountList.count() > 1) {
            text = tr("Pause all synchronization");
        } else {
            text = tr("Pause synchronization");
        }
        QAction *action = _contextMenu->addAction(text);
        connect(action, &QAction::triggered, this, &ownCloudGui::slotPauseAllFolders);
    }
    if (atLeastOneSignedIn) {
        if (accountList.count() > 1) {
            _actionLogout->setText(tr("Log out of all accounts"));
        } else {
            _actionLogout->setText(tr("Log out"));
        }
        _contextMenu->addAction(_actionLogout);
    }
    if (atLeastOneSignedOut) {
        if (accountList.count() > 1) {
            _actionLogin->setText(tr("Log in to all accounts..."));
        } else {
            _actionLogin->setText(tr("Log in..."));
        }
        _contextMenu->addAction(_actionLogin);
    }

    if (! Theme::instance()->about().isEmpty()) {
        _contextMenu->addSeparator();
        _contextMenu->addAction(_actionAbout);
    }

    _contextMenu->addAction(_actionQuit);
#endif

    if (_workaroundShowAndHideTray) {
        _tray->show();
    }
}

void ownCloudGui::updatePopoverNeeded()
{
    // if it's visible and we can update live: update now
#ifdef KDRIVE_V2
    updatePopover();
    return;
#else
    if (popoverVisible() && updateWhileVisible()) {
        // Note: don't update while visible on OSX
        // https://bugreports.qt.io/browse/QTBUG-54845
        updatePopover();
        return;
    }

    // if we can't lazily update: update later
    if (_workaroundNoAboutToShowUpdate) {
        // Note: don't update immediately even in the invisible case
        // as that can lead to extremely frequent menu updates
        if (!_delayedTrayUpdateTimer.isActive()) {
            _delayedTrayUpdateTimer.start();
        }
        return;
    }
#endif
}

void ownCloudGui::onRefreshAccountList()
{
    updatePopoverNeeded();
}

void ownCloudGui::slotShowTrayMessage(const QString &title, const QString &msg)
{
    if (_tray) {
        _tray->showMessage(title, msg);
    }
    else {
        qCWarning(lcApplication) << "Tray not ready: " << msg;
    }
}

void ownCloudGui::slotShowOptionalTrayMessage(const QString &title, const QString &msg)
{
    ConfigFile cfg;
    if (cfg.optionalDesktopNotifications()) {
        if (_notificationEnableDate == QDateTime()
                || _notificationEnableDate > QDateTime::currentDateTime()) {
            slotShowTrayMessage(title, msg);
        }
    }
}

/*
 * open the folder with the given Alias
 */
void ownCloudGui::slotFolderOpenAction(const QString &alias)
{
    Folder *f = FolderMan::instance()->folder(alias);
    if (f) {
        qCInfo(lcApplication) << "opening local url " << f->path();
        QUrl url = QUrl::fromLocalFile(f->path());

#ifdef Q_OS_WIN
        // work around a bug in QDesktopServices on Win32, see i-net
        QString filePath = f->path();

        if (filePath.startsWith(QLatin1String("\\\\")) || filePath.startsWith(QLatin1String("//")))
            url = QUrl::fromLocalFile(QDir::toNativeSeparators(filePath));
        else
            url = QUrl::fromLocalFile(filePath);
#endif
        QDesktopServices::openUrl(url);
    }
}

#ifndef KDRIVE_V2
void ownCloudGui::setupActions()
{
    _actionShowErrors = new QAction(Theme::instance()->stateErrorIcon(), tr("See synchronization errors"), this);
    _actionShowErrors->setEnabled(false);
    _actionStatus = new QAction(tr("Unknown status"), this);
    _actionStatus->setEnabled(false);
    _actionSettings = new QAction(tr("Settings..."), this);
    _actionNewAccountWizard = new QAction(tr("New account..."), this);
    _actionRecent = new QAction(tr("Details..."), this);
    _actionRecent->setEnabled(true);

    QObject::connect(_actionShowErrors, &QAction::triggered, this, &ownCloudGui::slotShowErrors);
    QObject::connect(_actionRecent, &QAction::triggered, this, &ownCloudGui::slotShowSyncProtocol);
    QObject::connect(_actionSettings, &QAction::triggered, this, &ownCloudGui::slotShowSettings);
    QObject::connect(_actionNewAccountWizard, &QAction::triggered, this, &ownCloudGui::slotNewAccountWizard);
    _actionHelp = new QAction(tr("Help"), this);
    QObject::connect(_actionHelp, &QAction::triggered, this, &ownCloudGui::slotHelp);
    _actionAbout = new QAction(tr("About %1").arg(Theme::instance()->appNameGUI()), this);
    QObject::connect(_actionAbout, &QAction::triggered, this, &ownCloudGui::slotAbout);
    _actionQuit = new QAction(tr("Quit %1").arg(Theme::instance()->appNameGUI()), this);
    QObject::connect(_actionQuit, SIGNAL(triggered(bool)), _app, SLOT(quit()));

    _actionLogin = new QAction(tr("Log in..."), this);
    connect(_actionLogin, &QAction::triggered, this, &ownCloudGui::slotLogin);
    _actionLogout = new QAction(tr("Log out"), this);
    connect(_actionLogout, &QAction::triggered, this, &ownCloudGui::slotLogout);

    if (_app->debugMode()) {
        _actionCrash = new QAction("Crash now - Div by zero", this);
        connect(_actionCrash, &QAction::triggered, _app, &Application::slotCrash);
        _actionCrashEnforce = new QAction("Crash now - ENFORCE()", this);
        connect(_actionCrashEnforce, &QAction::triggered, _app, &Application::slotCrashEnforce);
        _actionCrashFatal = new QAction("Crash now - qFatal", this);
        connect(_actionCrashFatal, &QAction::triggered, _app, &Application::slotCrashFatal);
    } else {
        _actionCrash = nullptr;
        _actionCrashEnforce = nullptr;
        _actionCrashFatal = nullptr;
    }
}

void ownCloudGui::slotRebuildRecentMenus()
{
    _recentActionsMenu->clear();
    if (!_recentItemsActions.isEmpty()) {
        foreach (QAction *a, _recentItemsActions) {
            _recentActionsMenu->addAction(a);
        }
        _recentActionsMenu->addSeparator();
    } else {
        _recentActionsMenu->addAction(tr("No items synced recently"))->setEnabled(false);
    }
    // add a more... entry.
    _recentActionsMenu->addAction(_actionRecent);
}

/// Returns true if the completion of a given item should show up in the
/// 'Recent Activity' menu
static bool shouldShowInRecentsMenu(const SyncFileItem &item)
{
    return !Progress::isIgnoredKind(item._status)
        && item._instruction != CSYNC_INSTRUCTION_EVAL
        && item._instruction != CSYNC_INSTRUCTION_NONE;
}
#endif

void ownCloudGui::slotUpdateProgress(const QString &folder, const ProgressInfo &progress)
{
#ifdef KDRIVE_V2
    Q_UNUSED(folder);

    if (_synthesisPopover) {
        emit _synthesisPopover->updateProgress(folder, progress);
    }
#else
    if (progress.status() == ProgressInfo::Discovery) {
        if (!progress._currentDiscoveredRemoteFolder.isEmpty()) {
            _actionStatus->setText(tr("Checking for changes in remote '%1'")
                                       .arg(progress._currentDiscoveredRemoteFolder));
        } else if (!progress._currentDiscoveredLocalFolder.isEmpty()) {
            _actionStatus->setText(tr("Checking for changes in local '%1'")
                                       .arg(progress._currentDiscoveredLocalFolder));
        }
    } else if (progress.status() == ProgressInfo::Done) {
        QTimer::singleShot(2000, this, &ownCloudGui::slotComputeOverallSyncStatus);
    }
    if (progress.status() != ProgressInfo::Propagation) {
        return;
    }

    if (progress.totalSize() == 0) {
        qint64 currentFile = progress.currentFile();
        qint64 totalFileCount = qMax(progress.totalFiles(), currentFile);
        QString msg;
        if (progress.trustEta()) {
            msg = tr("Syncing %1 of %2  (%3 left)")
                      .arg(currentFile)
                      .arg(totalFileCount)
                      .arg(Utility::durationToDescriptiveString2(progress.totalProgress().estimatedEta));
        } else {
            msg = tr("Syncing %1 of %2")
                      .arg(currentFile)
                      .arg(totalFileCount);
        }
        _actionStatus->setText(msg);
    } else {
        QString totalSizeStr = Utility::octetsToString(progress.totalSize());
        QString msg;
        if (progress.trustEta()) {
            msg = tr("Syncing %1 (%2 left)")
                      .arg(totalSizeStr, Utility::durationToDescriptiveString2(progress.totalProgress().estimatedEta));
        } else {
            msg = tr("Syncing %1")
                      .arg(totalSizeStr);
        }
        _actionStatus->setText(msg);
    }

    _actionRecent->setIcon(QIcon()); // Fixme: Set a "in-progress"-item eventually.

    if (!progress._lastCompletedItem.isEmpty()
        && shouldShowInRecentsMenu(progress._lastCompletedItem)) {
        if (Progress::isWarningKind(progress._lastCompletedItem._status)) {
            // display a warn icon if warnings happened.
            QIcon warnIcon(":/client/resources/warning");
            _actionRecent->setIcon(warnIcon);
        }

        QString kindStr = Progress::asResultString(progress._lastCompletedItem);
        QString timeStr = QTime::currentTime().toString("hh:mm");
        QString actionText = tr("%1 (%2, %3)").arg(progress._lastCompletedItem._file, kindStr, timeStr);
        QAction *action = new QAction(actionText, this);
        Folder *f = FolderMan::instance()->folder(folder);
        if (f) {
            QString fullPath = f->path() + '/' + progress._lastCompletedItem._file;
            if (QFile(fullPath).exists()) {
                connect(action, &QAction::triggered, this, [this, fullPath] { this->slotOpenPath(fullPath); });
            } else {
                action->setEnabled(false);
            }
        }
        if (_recentItemsActions.length() > 5) {
            _recentItemsActions.takeFirst()->deleteLater();
        }
        _recentItemsActions.append(action);

        // Update the "Recent" menu if the context menu is being shown,
        // otherwise it'll be updated later, when the context menu is opened.
        if (updateWhileVisible() && popoverVisible()) {
            slotRebuildRecentMenus();
        }
    }
#endif
}

void ownCloudGui::slotItemCompleted(const QString &folder, const SyncFileItemPtr &item)
{
    if (_synthesisPopover) {
         emit _synthesisPopover->itemCompleted(folder, item);
    }
}

#ifndef KDRIVE_V2
void ownCloudGui::slotLogin()
{
    if (auto account = qvariant_cast<AccountStatePtr>(sender()->property(propertyAccountC))) {
        account->account()->resetRejectedCertificates();
        account->signIn();
    } else {
        auto list = AccountManager::instance()->accounts();
        foreach (const auto &a, list) {
            a->signIn();
        }
    }
}

void ownCloudGui::slotLogout()
{
    auto list = AccountManager::instance()->accounts();
    if (auto account = qvariant_cast<AccountStatePtr>(sender()->property(propertyAccountC))) {
        list.clear();
        list.append(account);
    }

    foreach (const auto &ai, list) {
        ai->signOutByUi();
    }
}
#endif

void ownCloudGui::slotUnpauseAllFolders()
{
    setPauseOnAllFoldersHelper(false);
}

void ownCloudGui::slotPauseAllFolders()
{
    setPauseOnAllFoldersHelper(true);
}

void ownCloudGui::slotNewAccountWizard()
{
    OwncloudSetupWizard::runWizard(qApp, SLOT(slotownCloudWizardDone(int)));
}

void ownCloudGui::slotDisableNotifications(KDC::SynthesisPopover::NotificationsDisabled type,
                                           QDateTime value)
{
    ConfigFile cfg;
    if (type == KDC::SynthesisPopover::NotificationsDisabled::Never) {
        _notificationEnableDate = QDateTime();
        cfg.setOptionalDesktopNotifications(true);
    }
    else if (type == KDC::SynthesisPopover::NotificationsDisabled::Always) {
        _notificationEnableDate = QDateTime();
        cfg.setOptionalDesktopNotifications(false);
    }
    else {
        _notificationEnableDate = value;
        cfg.setOptionalDesktopNotifications(false);
    }
}

void ownCloudGui::slotApplyStyle()
{
    Utility::setStyle(qApp);
    slotComputeOverallSyncStatus();
}

void ownCloudGui::slotSetStyle(bool darkTheme)
{
    ConfigFile cfg;
    cfg.setDarkTheme(darkTheme);
    Utility::setStyle(qApp, darkTheme);
    slotComputeOverallSyncStatus();
}

void ownCloudGui::setPauseOnAllFoldersHelper(bool pause)
{
    QList<AccountState *> accounts;
    if (auto account = qvariant_cast<AccountStatePtr>(sender()->property(propertyAccountC))) {
        accounts.append(account.data());
    } else {
        foreach (auto a, AccountManager::instance()->accounts()) {
            accounts.append(a.data());
        }
    }
    foreach (Folder *f, FolderMan::instance()->map()) {
        if (accounts.contains(f->accountState())) {
            f->setSyncPaused(pause);
            if (pause) {
                f->slotTerminateSync();
            }
        }
    }
}

void ownCloudGui::slotShowGuiMessage(const QString &title, const QString &message)
{
    QMessageBox *msgBox = new QMessageBox;
    msgBox->setWindowFlags(msgBox->windowFlags() | Qt::WindowStaysOnTopHint);
    msgBox->setAttribute(Qt::WA_DeleteOnClose);
    msgBox->setText(message);
    msgBox->setWindowTitle(title);
    msgBox->setIcon(QMessageBox::Information);
    msgBox->open();
}

void ownCloudGui::slotShowParametersDialog()
{
#ifdef KDRIVE_V2_NEW_SETTINGS
    if (_parametersDialog.isNull()) {
        _parametersDialog = new KDC::ParametersDialog();
        connect(_parametersDialog, &KDC::ParametersDialog::addDrive, this, &ownCloudGui::slotNewAccountWizard);
        connect(_parametersDialog, &KDC::ParametersDialog::setStyle, this, &ownCloudGui::slotSetStyle);
        _parametersDialog->setAttribute(Qt::WA_DeleteOnClose, true);
    }
    raiseDialog(_parametersDialog);
#else
    if (_settingsDialog.isNull()) {
        _settingsDialog =
            new SettingsDialog(this);
        _settingsDialog->setAttribute(Qt::WA_DeleteOnClose, true);
        _settingsDialog->show();
    }
    raiseDialog(_settingsDialog.data());
#endif
}

#ifndef KDRIVE_V2
void ownCloudGui::slotShowSyncProtocol()
{
    slotShowSettings();
    _settingsDialog->showActivityPage();
}
#endif

void ownCloudGui::slotShutdown()
{
    // explicitly close windows. This is somewhat of a hack to ensure
    // that saving the geometries happens ASAP during a OS shutdown

    // those do delete on close
#ifdef KDRIVE_V2_NEW_SETTINGS
    if (!_parametersDialog.isNull())
        _parametersDialog->close();
#else
    if (!_settingsDialog.isNull())
        _settingsDialog->close();
#endif
    if (!_logBrowser.isNull())
        _logBrowser->deleteLater();
}

void ownCloudGui::slotToggleLogBrowser()
{
    if (_logBrowser.isNull()) {
        // init the log browser.
        _logBrowser = new LogBrowser;
        // ## TODO: allow new log name maybe?
    }

    if (_logBrowser->isVisible()) {
        _logBrowser->hide();
    } else {
        raiseDialog(_logBrowser);
    }
}

void ownCloudGui::slotOpenWebview()
{
    if (auto account = qvariant_cast<AccountPtr>(sender()->property(propertyAccountC))) {
        QDesktopServices::openUrl(account->url());
    }
}

void ownCloudGui::slotHelp()
{
    QDesktopServices::openUrl(QUrl(Theme::instance()->helpUrl()));
}

void ownCloudGui::raiseDialog(QWidget *raiseWidget)
{
    QWidget *activeWindow = QApplication::activeWindow();
    if (activeWindow && activeWindow != raiseWidget) {
        activeWindow->hide();
    }
    if (raiseWidget && !raiseWidget->parentWidget()) {
        // Qt has a bug which causes parent-less dialogs to pop-under.
        raiseWidget->showNormal();
        raiseWidget->raise();
        raiseWidget->activateWindow();

#if defined(Q_OS_X11)
        WId wid = widget->winId();
        NETWM::init();

        XEvent e;
        e.xclient.type = ClientMessage;
        e.xclient.message_type = NETWM::NET_ACTIVE_WINDOW;
        e.xclient.display = QX11Info::display();
        e.xclient.window = wid;
        e.xclient.format = 32;
        e.xclient.data.l[0] = 2;
        e.xclient.data.l[1] = QX11Info::appTime();
        e.xclient.data.l[2] = 0;
        e.xclient.data.l[3] = 0l;
        e.xclient.data.l[4] = 0l;
        Display *display = QX11Info::display();
        XSendEvent(display,
            RootWindow(display, DefaultScreen(display)),
            False, // propagate
            SubstructureRedirectMask | SubstructureNotifyMask,
            &e);
#endif
    }
}


void ownCloudGui::slotShowShareDialog(const QString &sharePath, const QString &localPath, ShareDialogStartPage startPage)
{
    QString file;
    const auto folder = FolderMan::instance()->folderForPath(localPath, &file);
    if (!folder) {
        qCWarning(lcApplication) << "Could not open share dialog for" << localPath << "no responsible folder found";
        return;
    }

    const auto accountState = folder->accountState();

    SyncJournalFileRecord fileRecord;

    bool resharingAllowed = true; // lets assume the good
    if (folder->journalDb()->getFileRecord(file, &fileRecord) && fileRecord.isValid()) {
        // check the permission: Is resharing allowed?
        if (!fileRecord._remotePerm.isNull() && !fileRecord._remotePerm.hasPermission(RemotePermissions::CanReshare)) {
            resharingAllowed = false;
        }
    }

    // As a first approximation, set the set of permissions that can be granted
    // either to everything (resharing allowed) or nothing (no resharing).
    //
    // The correct value will be found with a propfind from ShareDialog.
    // (we want to show the dialog directly, not wait for the propfind first)
    SharePermissions maxSharingPermissions =
        SharePermissionRead
        | SharePermissionUpdate | SharePermissionCreate | SharePermissionDelete
        | SharePermissionShare;
    if (!resharingAllowed) {
        maxSharingPermissions = SharePermission(0);
    }


    ShareDialog *w = nullptr;
    if (_shareDialogs.contains(localPath) && _shareDialogs[localPath]) {
        qCInfo(lcApplication) << "Raising share dialog" << sharePath << localPath;
        w = _shareDialogs[localPath];
    } else {
        qCInfo(lcApplication) << "Opening share dialog" << sharePath << localPath << maxSharingPermissions;
        w = new ShareDialog(accountState, sharePath, localPath, maxSharingPermissions, fileRecord.legacyDeriveNumericFileId(), startPage);
        w->setAttribute(Qt::WA_DeleteOnClose, true);

        _shareDialogs[localPath] = w;
        connect(w, &QObject::destroyed, this, &ownCloudGui::slotRemoveDestroyedShareDialogs);
    }
    raiseDialog(w);
}

void ownCloudGui::slotShowShareDialogPublicLinks(const QString &sharePath, const QString &localPath)
{
    slotShowShareDialog(sharePath, localPath, ShareDialogStartPage::PublicLinks);
}

void ownCloudGui::slotRemoveDestroyedShareDialogs()
{
    QMutableMapIterator<QString, QPointer<ShareDialog>> it(_shareDialogs);
    while (it.hasNext()) {
        it.next();
        if (!it.value() || it.value() == sender()) {
            it.remove();
        }
    }
}

void ownCloudGui::slotAbout()
{
    QString title = tr("About %1").arg(Theme::instance()->appNameGUI());
    QString about = Theme::instance()->about();
#ifdef KDRIVE_V2_NEW_SETTINGS
    QMessageBox *msgBox = new QMessageBox(this->_parametersDialog);
#else
    QMessageBox *msgBox = new QMessageBox(this->_settingsDialog);
#endif
#ifdef Q_OS_MAC
    // From Qt doc: "On macOS, the window title is ignored (as required by the macOS Guidelines)."
    msgBox->setText(title);
#else
    msgBox->setWindowTitle(title);
#endif
    msgBox->setAttribute(Qt::WA_DeleteOnClose, true);
    msgBox->setTextFormat(Qt::RichText);
    msgBox->setTextInteractionFlags(Qt::TextBrowserInteraction);
    msgBox->setInformativeText("<qt>"+about+"</qt>");
    msgBox->setStandardButtons(QMessageBox::Ok);
    QIcon appIcon = Theme::instance()->applicationIcon();
    // Assume icon is always small enough to fit an about dialog?
    qDebug() << appIcon.availableSizes().last();
    QPixmap iconPixmap = appIcon.pixmap(appIcon.availableSizes().last());
    iconPixmap.setDevicePixelRatio(2);
    msgBox->setIconPixmap(iconPixmap);
    msgBox->show();
}

} // end namespace
