#pragma once

#include "customtoolbutton.h"
#include "driveselectionwidget.h"
#include "progressbarwidget.h"
#include "statusbarwidget.h"
#include "synchronizeditemwidget.h"
#include "accountmanager.h"
#include "accountstate.h"
#include "quotainfo.h"
#include "folderman.h"
#include "progressdispatcher.h"

#include <QColor>
#include <QDialog>
#include <QEvent>
#include <QList>
#include <QListWidget>
#include <QRect>
#include <QStackedWidget>

namespace KDC {

class SynthesisPopover : public QDialog
{
    Q_OBJECT

    Q_PROPERTY(QColor background_main_color READ backgroundMainColor WRITE setBackgroundMainColor)

public:
    enum notificationActions {
        Never = 0,
        OneHour,
        UntilTomorrow,
        TreeDays,
        OneWeek,
        Always
    };

    explicit SynthesisPopover(bool debugMode, QRect sysrayIconRect, QWidget *parent = nullptr);

    inline QColor backgroundMainColor() const { return _backgroundMainColor; }
    inline void setBackgroundMainColor(const QColor &value) { _backgroundMainColor = value; }

signals:
    void refreshAccountList();
    void updateProgress(const QString &folderId, const OCC::ProgressInfo &progress);
    void itemCompleted(const QString &folderId, const OCC::SyncFileItemPtr &syncFileItemPtr);
    void openParametersDialog();
    void openShareDialogPublicLinks(const QString &sharePath, const QString &localPath);
    void openHelp();
    void exit();
    void crash();
    void crashEnforce();
    void crashFatal();

private:
    enum stackedWidget {
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
    QStackedWidget *_stackedWidget;
    notificationActions _notificationAction;
    std::map<QString, AccountStatus> _accountStatusMap;

    void paintEvent(QPaintEvent *event) override;
    bool event(QEvent *event) override;
    void initUI();
    OCC::SyncResult::Status computeAccountStatus(const std::map<QString, FolderInfo> &folderMap);
    void computeAccountProgress(const std::map<QString, FolderInfo> &folderMap,
                               qint64 &currentFile, qint64 &totalFiles,
                               qint64 &processedSize, qint64 &totalSize,
                               qint64 &estimatedRemainingTime);
    void pauseSync(bool pause);
    QString folderPath(const QString &folderId, const QString &filePath);
    QUrl folderUrl(const QString &folderId, const QString &filePath);
    void openUrl(const QString &folderId, const QString &filePath = QString());
    const SynchronizedItem *currentSynchronizedItem();

private slots:
    void onRefreshAccountList();
    void onUpdateProgress(const QString &folderId, const OCC::ProgressInfo &progress);
    void onUpdateQuota(qint64 total, qint64 used);
    void onItemCompleted(const QString &folderId, const OCC::SyncFileItemPtr &item);
    void onOpenFolderMenu(bool checked);
    void onOpenFolder(bool checked);
    void onOpenWebview(bool checked);
    void onOpenMenu(bool checked);
    void onOpenParameters(bool checked = false);
    void onNotificationActionTriggered(bool checked = false);
    void onDisplayHelp(bool checked = false);
    void onExit(bool checked = false);
    void onCrash(bool checked = false);
    void onCrashEnforce(bool checked = false);
    void onCrashFatal(bool checked = false);
    void onAccountSelected(QString id);
    void onPauseSync();
    void onResumeSync();
    void onRunSync();
    void onButtonBarToggled(int position);
    void onCurrentSynchronizedWidgetItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void onOpenFolderItem();
    void onOpenItem();
    void onAddToFavouriteItem();
    void onManageRightAndSharingItem();
    void onCopyLinkItem();
    void onOpenWebviewItem();
};

}

Q_DECLARE_METATYPE(KDC::SynthesisPopover::notificationActions)
