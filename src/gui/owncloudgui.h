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

#ifndef OWNCLOUDGUI_H
#define OWNCLOUDGUI_H

#define KDRIVE_V2

#include "systray.h"
#include "connectionvalidator.h"
#include "progressdispatcher.h"
#include "synthesispopover.h"

#include <QObject>
#include <QPointer>
#include <QAction>
#include <QMenu>
#include <QSize>
#include <QTimer>

namespace OCC {

class Folder;

class SettingsDialog;
class ShareDialog;
class Application;
class LogBrowser;
class AccountState;

enum class ShareDialogStartPage {
    UsersAndGroups,
    PublicLinks,
};

/**
 * @brief The ownCloudGui class
 * @ingroup gui
 */
class ownCloudGui : public QObject
{
    Q_OBJECT
public:
    explicit ownCloudGui(Application *parent = 0);

    bool checkAccountExists(bool openSettings);

    static void raiseDialog(QWidget *raiseWidget);
    static QSize settingsDialogSize() { return QSize(800, 500); }
    void setupOverlayIcons();

#ifndef KDRIVE_V2
    /// Whether the tray menu is visible
    bool popoverVisible() const;
#endif

    void hideAndShowTray();

signals:
    void setupProxy();

public slots:
    void setupPopover();
    void updatePopover();
    void updatePopoverNeeded();
#ifndef KDRIVE_V2
    void slotPopoverAboutToShow();
    void slotPopoverAboutToHide();
#endif
    void slotComputeOverallSyncStatus();
    void slotShowTrayMessage(const QString &title, const QString &msg);
    void slotShowOptionalTrayMessage(const QString &title, const QString &msg);
    void slotFolderOpenAction(const QString &alias);
#ifndef KDRIVE_V2
    void slotRebuildRecentMenus();
#endif
    void slotUpdateProgress(const QString &folder, const ProgressInfo &progress);
    void slotItemCompleted(const QString &folder, const SyncFileItemPtr &item);
    void slotShowGuiMessage(const QString &title, const QString &message);
    void slotFoldersChanged();
    void slotShowSettings();
    void slotShowSyncProtocol();
    void slotShutdown();
    void slotSyncStateChange(Folder *);
    void slotTrayClicked(QSystemTrayIcon::ActivationReason reason);
    void slotToggleLogBrowser();
    void slotOpenWebview();
    void slotOpenSettingsDialog();
    void slotHelp();
    void slotAbout();
    void slotOpenPath(const QString &path);
    void slotAccountStateChanged();
    void slotTrayMessageIfServerUnsupported(Account *account);
    void slotShowErrors();

    /**
     * Open a share dialog for a file or folder.
     *
     * sharePath is the full remote path to the item,
     * localPath is the absolute local path to it (so not relative
     * to the folder).
     */
    void slotShowShareDialog(const QString &sharePath, const QString &localPath, ShareDialogStartPage startPage);
    void slotShowShareDialogPublicLinks(const QString &sharePath, const QString &localPath);

    void slotRemoveDestroyedShareDialogs();

private slots:
#ifndef KDRIVE_V2
    void slotLogin();
    void slotLogout();
#endif
    void slotUnpauseAllFolders();
    void slotPauseAllFolders();
    void slotNewAccountWizard();
    void slotDisableNotifications(KDC::SynthesisPopover::NotificationsDisabled type, QDateTime value);

private:
    void setPauseOnAllFoldersHelper(bool pause);
#ifndef KDRIVE_V2
    void setupActions();
    void addAccountContextMenu(AccountStatePtr accountState, QMenu *menu, bool separateMenu);
#endif

    QPointer<Systray> _tray;
    QPointer<SettingsDialog> _settingsDialog;
    QPointer<LogBrowser> _logBrowser;
#ifdef KDRIVE_V2
    QScopedPointer<KDC::SynthesisPopover> _synthesisPopover;
#else
    // tray's menu
    QScopedPointer<QMenu> _contextMenu;
#endif

#ifndef KDRIVE_V2
    // Manually tracking whether the context menu is visible via aboutToShow
    // and aboutToHide. Unfortunately aboutToHide isn't reliable everywhere
    // so this only gets used with _workaroundManualVisibility (when the tray's
    // isVisible() is unreliable)
    bool _popoverVisibleManual = false;
#endif

    QMenu *_recentActionsMenu;
    QVector<QMenu *> _accountMenus;
    bool _workaroundShowAndHideTray = false;
    bool _workaroundNoAboutToShowUpdate = false;
    bool _workaroundFakeDoubleClick = false;
    bool _workaroundManualVisibility = false;
    QTimer _delayedTrayUpdateTimer;
    QMap<QString, QPointer<ShareDialog>> _shareDialogs;
    QDateTime _notificationEnableDate;

    QAction *_actionLogin;
    QAction *_actionLogout;

    QAction *_actionNewAccountWizard;
    QAction *_actionSettings;
    QAction *_actionShowErrors;
    QAction *_actionStatus;
    QAction *_actionEstimate;
    QAction *_actionRecent;
    QAction *_actionHelp;
    QAction *_actionAbout;
    QAction *_actionQuit;
    QAction *_actionCrash;
    QAction *_actionCrashEnforce;
    QAction *_actionCrashFatal;


    QList<QAction *> _recentItemsActions;
    Application *_app;
};

} // namespace OCC

#endif // OWNCLOUDGUI_H
