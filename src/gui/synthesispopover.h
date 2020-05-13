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
#include "accountinfo.h"
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

    void setPosition(const QRect &sysTrayIconRect);

signals:
    void updateProgress(const QString &folderId, const OCC::ProgressInfo &progress);
    void itemCompleted(const QString &folderId, const OCC::SyncFileItemPtr &syncFileItemPtr);
    void openParametersDialog(const QString &accountId = QString());
    void openShareDialogPublicLinks(const QString &sharePath, const QString &localPath);
    void exit();
    void addDrive();
    void disableNotifications(NotificationsDisabled type, QDateTime dateTime);
    void applyStyle();
    void crash();
    void crashEnforce();
    void crashFatal();

private:
    enum StackedWidget {
        Synchronized = 0,
        Favorites,
        Activity
    };

    struct AccountInfoSynthesis : public AccountInfo {
        StackedWidget _stackedWidgetPosition;
        QListWidget *_synchronizedListWidget;
        int _synchronizedListStackPosition;
        int _favoritesListStackPosition;
        int _activityListStackPosition;

        AccountInfoSynthesis();
        AccountInfoSynthesis(OCC::AccountState *accountState);
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
    QWidget *_defaultSynchronizedPageWidget;
    NotificationsDisabled _notificationsDisabled;
    QDateTime _notificationsDisabledUntilDateTime;
    std::map<QString, AccountInfoSynthesis> _accountInfoMap;

    void changeEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    bool event(QEvent *event) override;

    inline QColor backgroundMainColor() const { return _backgroundMainColor; }
    inline void setBackgroundMainColor(const QColor &value) { _backgroundMainColor = value; }

    void initUI();
    QString folderPath(const QString &folderId, const QString &filePath);
    QUrl folderUrl(const QString &folderId, const QString &filePath);
    void openUrl(const QString &folderId, const QString &filePath = QString());
    const FolderInfo *getFirstFolderWithStatus(const std::map<QString, FolderInfo *> &folderMap,
                                               OCC::SyncResult::Status status);
    const FolderInfo *getFirstFolderByPriority(const std::map<QString, FolderInfo *> &folderMap);
    void refreshStatusBar(const FolderInfo *folderInfo);
    void refreshStatusBar(std::map<QString, AccountInfoSynthesis>::iterator accountStatusIt);
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
    void onOpenFolderItem(const SynchronizedItem &item);
    void onOpenItem(const SynchronizedItem &item);
    void onAddToFavouriteItem(const SynchronizedItem &item);
    void onManageRightAndSharingItem(const SynchronizedItem &item);
    void onCopyLinkItem(const SynchronizedItem &item);
    void onOpenWebviewItem(const SynchronizedItem &item);
    void onCopyUrlToClipboard(const QString &url);
    void onLinkActivated(const QString &link);
};

}

Q_DECLARE_METATYPE(KDC::SynthesisPopover::NotificationsDisabled)
