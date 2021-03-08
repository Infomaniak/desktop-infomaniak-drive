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

#include "systray.h"
#include "connectionvalidator.h"
#include "progressdispatcher.h"
#include "synthesispopover.h"
#include "parametersdialog.h"

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
class AccountState;

enum class ShareDialogStartPage {
    UsersAndGroups,
    PublicLinks,
};

/**
 * @brief The OwnCloudGui class
 * @ingroup gui
 */
class OwnCloudGui : public QObject
{
    Q_OBJECT
public:
    explicit OwnCloudGui(Application *parent = 0);

    void hideAndShowTray();
    void showSynthesisDialog();
    int driveErrorCount(const QString &accountId) const;

public slots:
    void slotComputeOverallSyncStatus();
    void slotShowTrayMessage(const QString &title, const QString &msg);
    void slotShowGuiMessage(const QString &title, const QString &message);
    void slotShowParametersDialog(const QString &accountId = QString(), bool errorPage = false);
    void slotShutdown();
    void slotOpenParametersDialog(const QString &accountId = QString());
    void slotAccountStateChanged();
    void slotTrayMessageIfServerUnsupported(Account *account);

    /**
     * Open a share dialog for a file or folder.
     *
     * sharePath is the full remote path to the item,
     * localPath is the absolute local path to it (so not relative
     * to the folder).
     */
    void slotShowShareDialog(const QString &sharePath, const QString &localPath, ShareDialogStartPage startPage);

private slots:
    void slotUpdateSystray();
    void onRefreshAccountList();
    void slotShowOptionalTrayMessage(const QString &title, const QString &msg);
    void slotSyncStateChange(Folder *);
    void slotTrayClicked(QSystemTrayIcon::ActivationReason reason);
    void slotShowShareDialogPublicLinks(const QString &sharePath, const QString &localPath);
    void slotRemoveDestroyedShareDialogs();
    void slotNewAccountWizard();
    void slotDisableNotifications(KDC::SynthesisPopover::NotificationsDisabled type, QDateTime value);
    void slotApplyStyle();
    void slotSetStyle(bool darkTheme);

private:
    QScopedPointer<Systray> _tray;
    QScopedPointer<KDC::SynthesisPopover> _synthesisPopover;
    QScopedPointer<KDC::ParametersDialog> _parametersDialog;
    bool _workaroundShowAndHideTray = false;
    bool _workaroundNoAboutToShowUpdate = false;
    bool _workaroundFakeDoubleClick = false;
    bool _workaroundManualVisibility = false;
    QTimer _delayedTrayUpdateTimer;
    QMap<QString, QPointer<ShareDialog>> _shareDialogs;
    QDateTime _notificationEnableDate;
    bool _addDriveWizardRunning;
    Application *_app;

    static void raiseDialog(QWidget *raiseWidget);
    void setupSynthesisPopover();
    void setupParametersDialog();
    void updateSystrayNeeded();
};

} // namespace OCC

#endif // OWNCLOUDGUI_H
