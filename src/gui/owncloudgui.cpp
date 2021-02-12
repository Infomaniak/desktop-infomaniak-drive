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
#include "adddrivewizard.h"
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
#include "custommessagebox.h"

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

OwnCloudGui::OwnCloudGui(Application *parent)
    : QObject(parent)
    , _tray(nullptr)
    , _logBrowser(nullptr)
    , _recentActionsMenu(nullptr)
    , _notificationEnableDate(QDateTime())
    , _addDriveWizardRunning(false)
    , _app(parent)
{
    _tray = new Systray();
    _tray->setParent(this);

    // for the beginning, set the offline icon until the account was verified
    _tray->setIcon(Theme::instance()->folderOfflineIcon(/*systray?*/ true, /*currently visible?*/ false));

    connect(_tray.data(), &QSystemTrayIcon::activated, this, &OwnCloudGui::slotTrayClicked);

    _tray->show();
    setupSynthesisPopover();
    setupParametersDialog();

#ifdef WITH_LIBCLOUDPROVIDERS
    auto exporter = new LibCloudProviders(this);
    exporter->start();
    connect(exporter, &LibCloudProviders::showSettings, this, &ownCloudGui::slotShowSettings);
#endif

    ProgressDispatcher *pd = ProgressDispatcher::instance();

    connect(pd, &ProgressDispatcher::progressInfo, this, &OwnCloudGui::slotUpdateProgress);
    connect(pd, &ProgressDispatcher::itemCompleted, this, &OwnCloudGui::slotItemCompleted);

    FolderMan *folderMan = FolderMan::instance();
    connect(folderMan, &FolderMan::folderSyncStateChange, this, &OwnCloudGui::slotSyncStateChange);

    connect(AccountManager::instance(), &AccountManager::accountAdded, this, &OwnCloudGui::onRefreshAccountList);
    connect(AccountManager::instance(), &AccountManager::accountRemoved, this, &OwnCloudGui::onRefreshAccountList);

    connect(Logger::instance(), &Logger::guiLog, this, &OwnCloudGui::slotShowTrayMessage);
    connect(Logger::instance(), &Logger::optionalGuiLog, this, &OwnCloudGui::slotShowOptionalTrayMessage);
    connect(Logger::instance(), &Logger::guiMessage, this, &OwnCloudGui::slotShowGuiMessage);

    if (_tray->geometry().width() == 0) {
        // No systray
        qCDebug(lcApplication) << "No systray!";

        // Workaround: create fake menu
        QMenu *menu = new QMenu;
        _tray->setContextMenu(menu);
    }
}

// This should rather be in application.... or rather in ConfigFile?
void OwnCloudGui::slotOpenParametersDialog(const QString &accountId)
{
    // if account is set up, start the configuration wizard.
    if (!AccountManager::instance()->accounts().isEmpty()) {
        if (_parametersDialog.isNull() || QApplication::activeWindow() != _parametersDialog) {
            slotShowParametersDialog(accountId);
        } else {
            _parametersDialog->close();
        }
    } else {
        qCInfo(lcApplication) << "No configured folders yet, starting setup wizard";
        slotNewAccountWizard();
    }
}

void OwnCloudGui::slotTrayClicked(QSystemTrayIcon::ActivationReason reason)
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
        /*if (OwncloudSetupWizard::bringWizardToFrontIfVisible()) {
            // brought wizard to front
        } else*/ if (_shareDialogs.size() > 0) {
            // Share dialog(s) be hidden by other apps, bring them back
            Q_FOREACH (const QPointer<ShareDialog> &shareDialog, _shareDialogs) {
                Q_ASSERT(shareDialog.data());
                raiseDialog(shareDialog);
            }
        } else {
            showSynthesisDialog();
        }
    }
    // FIXME: Also make sure that any auto updater dialogue https://github.com/owncloud/client/issues/5613
    // or SSL error dialog also comes to front.
}

void OwnCloudGui::slotSyncStateChange(Folder *folder)
{
    slotComputeOverallSyncStatus();
    updatePopoverNeeded();

    if (!folder) {
        return; // Valid, just a general GUI redraw was needed.
    }

    auto result = folder->syncResult();

    qCInfo(lcApplication) << "Sync state changed for folder " << folder->remoteUrl().toString() << ": " << result.statusString();

    /*if (result.status() == SyncResult::Success
        || result.status() == SyncResult::Problem
        || result.status() == SyncResult::SyncAbortRequested
        || result.status() == SyncResult::Error) {
        Logger::instance()->enterNextLogFile();
    }*/
}

void OwnCloudGui::slotFoldersChanged()
{
    slotComputeOverallSyncStatus();
    updatePopoverNeeded();
}

void OwnCloudGui::slotOpenPath(const QString &path)
{
    showInFileManager(path);
}

void OwnCloudGui::slotAccountStateChanged()
{
    updatePopoverNeeded();
    slotComputeOverallSyncStatus();
}

void OwnCloudGui::slotTrayMessageIfServerUnsupported(Account *account)
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

void OwnCloudGui::slotShowErrors()
{
    slotShowParametersDialog();
}

void OwnCloudGui::slotComputeOverallSyncStatus()
{
    bool allSignedOut = true;
    bool allPaused = true;
    bool allDisconnected = true;
    QVector<AccountStatePtr> problemAccounts;

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
        _tray->setIcon(Theme::instance()->folderOfflineIcon(true, true));
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
        _tray->setIcon(Theme::instance()->folderOfflineIcon(true, true));
        _tray->setToolTip(tr("Please sign in"));
        return;
    } else if (allPaused) {
        _tray->setIcon(Theme::instance()->syncStateIcon(SyncResult::Paused, true, true));
        _tray->setToolTip(tr("Account synchronization is disabled"));
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
    QIcon statusIcon = Theme::instance()->syncStateIcon(iconStatus, true, true, app->getAlert());
    _tray->setIcon(statusIcon);

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
    } else {
        _tray->setToolTip(tr("There are no sync folders configured."));
    }
}

void OwnCloudGui::hideAndShowTray()
{
    _tray->hide();
    _tray->show();
}

void OwnCloudGui::showSynthesisDialog()
{
    if (_synthesisPopover) {
        if (_synthesisPopover.get()->isVisible()) {
            _synthesisPopover.get()->done(QDialog::Accepted);
        }
        else {
            QRect trayIconRect = _tray->geometry();
            if (!trayIconRect.isValid()) {
                trayIconRect = QRect(QCursor::pos(), QSize(0, 0));
            }
            _synthesisPopover->setPosition(trayIconRect);
            raiseDialog(_synthesisPopover.get());
        }
    }
}

int OwnCloudGui::driveErrorCount(const QString &accountId) const
{
    if (_parametersDialog) {
        return _parametersDialog->driveErrorCount(accountId);
    }
    return 0;
}

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

void OwnCloudGui::setupSynthesisPopover()
{
    if (_synthesisPopover) {
        return;
    }

    _synthesisPopover.reset(new KDC::SynthesisPopover(_app->debugMode(), this));
    connect(_synthesisPopover.get(), &KDC::SynthesisPopover::openParametersDialog, this, &OwnCloudGui::slotShowParametersDialog);
    connect(_synthesisPopover.get(), &KDC::SynthesisPopover::openShareDialogPublicLinks, this, &OwnCloudGui::slotShowShareDialogPublicLinks);
    connect(_synthesisPopover.get(), &KDC::SynthesisPopover::exit, _app, &Application::quit);
    connect(_synthesisPopover.get(), &KDC::SynthesisPopover::addDrive, this, &OwnCloudGui::slotNewAccountWizard);
    connect(_synthesisPopover.get(), &KDC::SynthesisPopover::disableNotifications, this, &OwnCloudGui::slotDisableNotifications);
    connect(_synthesisPopover.get(), &KDC::SynthesisPopover::applyStyle, this, &OwnCloudGui::slotApplyStyle);
    connect(_synthesisPopover.get(), &KDC::SynthesisPopover::crash, _app, &Application::slotCrash);
    connect(_synthesisPopover.get(), &KDC::SynthesisPopover::crashEnforce, _app, &Application::slotCrashEnforce);
    connect(_synthesisPopover.get(), &KDC::SynthesisPopover::crashFatal, _app, &Application::slotCrashFatal);

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

    applyEnvVariable(&_workaroundNoAboutToShowUpdate, envForceWorkaroundNoAboutToShowUpdate());
    applyEnvVariable(&_workaroundFakeDoubleClick, envForceWorkaroundFakeDoubleClick());
    applyEnvVariable(&_workaroundShowAndHideTray, envForceWorkaroundShowAndHideTray());
    applyEnvVariable(&_workaroundManualVisibility, envForceWorkaroundManualVisibility());

    qCInfo(lcApplication) << "Tray menu workarounds:"
                          << "noabouttoshow:" << _workaroundNoAboutToShowUpdate
                          << "fakedoubleclick:" << _workaroundFakeDoubleClick
                          << "showhide:" << _workaroundShowAndHideTray
                          << "manualvisibility:" << _workaroundManualVisibility;

    connect(&_delayedTrayUpdateTimer, &QTimer::timeout, this, &OwnCloudGui::updatePopover);
    _delayedTrayUpdateTimer.setInterval(2 * 1000);
    _delayedTrayUpdateTimer.setSingleShot(true);

    // Populate the context menu now.
    updatePopover();
}

void OwnCloudGui::setupParametersDialog()
{
    _parametersDialog = new KDC::ParametersDialog();
    connect(_parametersDialog, &KDC::ParametersDialog::addDrive, this, &OwnCloudGui::slotNewAccountWizard);
    connect(_parametersDialog, &KDC::ParametersDialog::setStyle, this, &OwnCloudGui::slotSetStyle);
}

void OwnCloudGui::updatePopover()
{
    if (_workaroundShowAndHideTray) {
        // To make tray menu updates work with these bugs (see setupPopover)
        // we need to hide and show the tray icon. We don't want to do that
        // while it's visible!
        if (!_delayedTrayUpdateTimer.isActive()) {
            _delayedTrayUpdateTimer.start();
        }
        _tray->hide();
    }

    if (_workaroundShowAndHideTray) {
        _tray->show();
    }
}

void OwnCloudGui::updatePopoverNeeded()
{
    // if it's visible and we can update live: update now
    updatePopover();
    return;
}

void OwnCloudGui::onRefreshAccountList()
{
    updatePopoverNeeded();
}

void OwnCloudGui::slotShowTrayMessage(const QString &title, const QString &msg)
{
    if (_tray) {
        _tray->showMessage(title, msg);
    }
    else {
        qCWarning(lcApplication) << "Tray not ready: " << msg;
    }
}

void OwnCloudGui::slotShowOptionalTrayMessage(const QString &title, const QString &msg)
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
void OwnCloudGui::slotFolderOpenAction(const QString &alias)
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

void OwnCloudGui::slotUpdateProgress(const QString &folder, const ProgressInfo &progress)
{
    Q_UNUSED(folder);
    Q_UNUSED(progress);
}

void OwnCloudGui::slotItemCompleted(const QString &folder, const SyncFileItemPtr &item)
{
    Q_UNUSED(folder)
    Q_UNUSED(item)
}

void OwnCloudGui::slotUnpauseAllFolders()
{
    setPauseOnAllFoldersHelper(false);
}

void OwnCloudGui::slotPauseAllFolders()
{
    setPauseOnAllFoldersHelper(true);
}

void OwnCloudGui::slotNewAccountWizard()
{
    if (_addDriveWizardRunning) {
        return;
    }

    _addDriveWizardRunning = true;
    KDC::AddDriveWizard *wizard = new KDC::AddDriveWizard();
    int res = wizard->exec();
    _addDriveWizardRunning = false;

    Application *app = qobject_cast<Application *>(qApp);
    app->slotownCloudWizardDone(res);

    // Run next action
    switch (wizard->nextAction()) {
    case Utility::WizardAction::OpenFolder:
        if (!Utility::openFolder(wizard->localFolderPath())) {
            KDC::CustomMessageBox *msgBox = new KDC::CustomMessageBox(
                        QMessageBox::Warning,
                        tr("Unable to open folder path %1.").arg(wizard->localFolderPath()),
                        QMessageBox::Ok);
            msgBox->exec();
        }
        break;
    case Utility::WizardAction::OpenParameters:
        slotShowParametersDialog(wizard->accountId());
        break;
    case Utility::WizardAction::AddDrive:
        slotNewAccountWizard();
        break;
    }
}

void OwnCloudGui::slotDisableNotifications(KDC::SynthesisPopover::NotificationsDisabled type,
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

void OwnCloudGui::slotApplyStyle()
{
    Utility::setStyle(qApp);
}

void OwnCloudGui::slotSetStyle(bool darkTheme)
{
    ConfigFile cfg;
    cfg.setDarkTheme(darkTheme);
    Utility::setStyle(qApp, darkTheme);

    // Force apply style
    _parametersDialog->forceRedraw();
    _synthesisPopover->forceRedraw();
}

void OwnCloudGui::setPauseOnAllFoldersHelper(bool pause)
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

void OwnCloudGui::slotShowGuiMessage(const QString &title, const QString &message)
{
    QMessageBox *msgBox = new QMessageBox;
    msgBox->setWindowFlags(msgBox->windowFlags() | Qt::WindowStaysOnTopHint);
    msgBox->setAttribute(Qt::WA_DeleteOnClose);
    msgBox->setText(message);
    msgBox->setWindowTitle(title);
    msgBox->setIcon(QMessageBox::Information);
    msgBox->open();
}

void OwnCloudGui::slotShowParametersDialog(const QString &accountId, bool errorPage)
{
    if (_parametersDialog.isNull()) {
        setupParametersDialog();
    }
    if (accountId.isEmpty()) {
        _parametersDialog->openPreferencesPage();
    }
    else {
        if (errorPage) {
            _parametersDialog->openDriveErrorsPage(accountId);
        }
        else {
            _parametersDialog->openDriveParametersPage(accountId);
        }
    }
    raiseDialog(_parametersDialog);
}

void OwnCloudGui::slotShutdown()
{
    // explicitly close windows. This is somewhat of a hack to ensure
    // that saving the geometries happens ASAP during a OS shutdown

    // those do delete on close
    if (!_parametersDialog.isNull())
        _parametersDialog->close();
    if (!_logBrowser.isNull())
        _logBrowser->deleteLater();
}

void OwnCloudGui::slotToggleLogBrowser()
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

void OwnCloudGui::slotOpenWebview()
{
    if (auto account = qvariant_cast<AccountPtr>(sender()->property(propertyAccountC))) {
        QDesktopServices::openUrl(account->url());
    }
}

void OwnCloudGui::slotHelp()
{
    QDesktopServices::openUrl(QUrl(Theme::instance()->helpUrl()));
}

void OwnCloudGui::raiseDialog(QWidget *raiseWidget)
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


void OwnCloudGui::slotShowShareDialog(const QString &sharePath, const QString &localPath, ShareDialogStartPage startPage)
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
        connect(w, &QObject::destroyed, this, &OwnCloudGui::slotRemoveDestroyedShareDialogs);
    }
    raiseDialog(w);
}

void OwnCloudGui::slotShowShareDialogPublicLinks(const QString &sharePath, const QString &localPath)
{
    slotShowShareDialog(sharePath, localPath, ShareDialogStartPage::PublicLinks);
}

void OwnCloudGui::slotRemoveDestroyedShareDialogs()
{
    QMutableMapIterator<QString, QPointer<ShareDialog>> it(_shareDialogs);
    while (it.hasNext()) {
        it.next();
        if (!it.value() || it.value() == sender()) {
            it.remove();
        }
    }
}

} // end namespace
