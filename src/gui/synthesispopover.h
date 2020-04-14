#pragma once

#include "customtoolbutton.h"
#include "driveselectionwidget.h"
#include "progressbarwidget.h"
#include "statusbarwidget.h"

#include <QColor>
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
    enum notificationActions {
        Never = 0,
        OneHour,
        UntilTomorrow,
        TreeDays,
        OneWeek,
        Always
    };

    explicit SynthesisPopover(QWidget *parent = nullptr);

    inline QColor backgroundMainColor() const { return _backgroundMainColor; }
    inline void setBackgroundMainColor(const QColor &value) { _backgroundMainColor = value; }

    void setSysTrayIconRect(const QRect &sysTrayIconRect);
    void setTransferTotalSize(long size);
    void setTransferSize(long size);
    void setStatus(OCC::SyncResult::Status status, int fileNum, int fileCount, const QTime &time);

private:
    enum stackedWidget {
        SynchronizedItems = 0,
        Favorites,
        Activity
    };

    QRect _sysTrayIconRect;
    QColor _backgroundMainColor;
    CustomToolButton *_folderButton;
    CustomToolButton *_webviewButton;
    CustomToolButton *_menuButton;
    DriveSelectionWidget *_driveSelectionWidget;
    ProgressBarWidget *_progressBarWidget;
    StatusBarWidget *_statusBarWidget;
    QStackedWidget *_stackedWidget;
    QListWidget *_synchronizedListWidget;
    notificationActions _notificationAction;

    void paintEvent(QPaintEvent *event) override;
    bool event(QEvent *event) override;
    void init();
    void load();
    void loadDriveList();
    void loadSynchronizedList();

private slots:
    void onFolderButtonClicked();
    void onWebviewButtonClicked();
    void onMenuButtonClicked();
    void onParametersActionTriggered(bool checked = false);
    void onNotificationActionTriggered(bool checked = false);
    void onHelpActionTriggered(bool checked = false);
    void onExitActionTriggered(bool checked = false);
    void onDriveSelected(int id);
    void onButtonBarToggled(int position);
    void onCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
};

}

Q_DECLARE_METATYPE(KDC::SynthesisPopover::notificationActions)
