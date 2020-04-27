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

#pragma once

#include "customtoolbutton.h"
#include "driveselectionwidget.h"
#include "progressbarwidget.h"
#include "statusbarwidget.h"
#include "buttonsbarwidget.h"
#include "synchronizeditemwidget.h"
#include "accountmanager.h"
#include "accountstate.h"
#include "quotainfo.h"
#include "folderman.h"
#include "progressdispatcher.h"

#include <QColor>
#include <QDateTime>
#include <QDialog>
#include <QEvent>
#include <QListWidget>
#include <QRect>
#include <QStackedWidget>

namespace KDC {

class SynthesisPopover : public QDialog
{
    Q_OBJECT

    Q_PROPERTY(QColor background_main_color READ backgroundMainColor WRITE setBackgroundMainColor)

public:
    enum NotificationsDisabled {
        Never = 0,
        OneHour,
        UntilTomorrow,
        TreeDays,
        OneWeek,
        Always
    };

    static const std::map<NotificationsDisabled, QString> _notificationsDisabledMap;
    static const std::map<NotificationsDisabled, QString> _notificationsDisabledForPeriodMap;

    explicit SynthesisPopover(bool debugMode, QWidget *parent = nullptr);

    inline QColor backgroundMainColor() const { return _backgroundMainColor; }
    inline void setBackgroundMainColor(const QColor &value) { _backgroundMainColor = value; }
    void setPosition(const QRect &sysTrayIconRect);

signals:
    void refreshAccountList();
    void updateProgress(const QString &folderId, const OCC::ProgressInfo &progress);
    void itemCompleted(const QString &folderId, const OCC::SyncFileItemPtr &syncFileItemPtr);
    void openParametersDialog();
    void openShareDialogPublicLinks(const QString &sharePath, const QString &localPath);
    void openHelp();
    void exit();
    void addDrive();
    void disableNotifications(NotificationsDisabled type, QDateTime dateTime);
    void crash();
    void crashEnforce();
    void crashFatal();

private:
    enum StackedWidget {
        Synchronized = 0,
        Favorites,
        Activity
    };

    struct FolderInfo {
        QString _name;
        QString _path;
        qint64 _currentFile;
        qint64 _totalFiles;
        qint64 _completedSize;
        qint64 _totalSize;
        qint64 _estimatedRemainingTime;
        bool _syncPaused;
        OCC::SyncResult::Status _status;

        FolderInfo(const QString &name = QString(), const QString &path = QString());
    };

    struct AccountStatus {
        std::shared_ptr<OCC::QuotaInfo> _quotaInfoPtr;
        qint64 _totalSize;
        qint64 _used;
        OCC::SyncResult::Status _status;
        std::map<QString, FolderInfo> _folderMap;
        StackedWidget _stackedWidgetPosition;
        QListWidget *_synchronizedListWidget;
        QListWidgetItem *_currentSynchronizedWidgetItem;
        int _synchronizedListStackPosition;
        int _favoritesListStackPosition;
        int _activityListStackPosition;

        AccountStatus(OCC::AccountState *accountState = nullptr);
    };

    bool _debugMode;
    QRect _sysTrayIconRect;
    QString _currentAccountId;
    QColor _backgroundMainColor;
    CustomToolButton *_folderButton;
    CustomToolButton *_webviewButton;
    CustomToolButton *_menuButton;
    DriveSelectionWidget *_driveSelectionWidget;
    ProgressBarWidget *_progressBarWidget;
    StatusBarWidget *_statusBarWidget;
    ButtonsBarWidget *_buttonsBarWidget;
    QStackedWidget *_stackedWidget;
    QWidget *_defaultSynchronizedPage;
    NotificationsDisabled _notificationsDisabled;
    QDateTime _notificationsDisabledUntilDateTime;
    std::map<QString, AccountStatus> _accountStatusMap;

    void changeEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    bool event(QEvent *event) override;
    void initUI();
    OCC::SyncResult::Status computeAccountStatus(const std::map<QString, FolderInfo> &folderMap);
    void pauseSync(bool all, bool pause);
    QString folderPath(const QString &folderId, const QString &filePath);
    QUrl folderUrl(const QString &folderId, const QString &filePath);
    void openUrl(const QString &folderId, const QString &filePath = QString());
    const SynchronizedItem *currentSynchronizedItem();
    const FolderInfo *getActiveFolder(const std::map<QString, FolderInfo> &folderMap);
    void refreshStatusBar(const FolderInfo *folderInfo);
    void refreshStatusBar(std::map<QString, AccountStatus>::iterator accountStatusIt);
    void refreshStatusBar(QString accountId);
    void setSynchronizedDefaultPage(QWidget **widget, QWidget *parent);

private slots:
    void onRefreshAccountList();
    void onUpdateProgress(const QString &folderId, const OCC::ProgressInfo &progress);
    void onUpdateQuota(qint64 total, qint64 used);
    void onItemCompleted(const QString &folderId, const OCC::SyncFileItemPtr &item);
    void onOpenFolderMenu(bool checked);
    void onOpenFolder(bool checked);
    void onOpenWebview(bool checked);
    void onOpenMiscellaneousMenu(bool checked);
    void onOpenParameters(bool checked = false);
    void onNotificationActionTriggered(bool checked = false);
    void onDisplayHelp(bool checked = false);
    void onExit(bool checked = false);
    void onCrash(bool checked = false);
    void onCrashEnforce(bool checked = false);
    void onCrashFatal(bool checked = false);
    void onAccountSelected(QString id);
    void onAddDrive();
    void onPauseSync(bool all);
    void onResumeSync(bool all);
    void onRunSync(bool all);
    void onButtonBarToggled(int position);
    void onCurrentSynchronizedWidgetItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void onOpenFolderItem();
    void onOpenItem();
    void onAddToFavouriteItem();
    void onManageRightAndSharingItem();
    void onCopyLinkItem();
    void onOpenWebviewItem();
    void onCopyUrlToClipboard(const QString &url);
};

}

Q_DECLARE_METATYPE(KDC::SynthesisPopover::NotificationsDisabled)
