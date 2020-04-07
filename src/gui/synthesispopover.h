#pragma once

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
    Q_PROPERTY(QColor border_color READ borderColor WRITE setBorderColor)

public:
    explicit SynthesisPopover(QWidget *parent = nullptr);

    inline QColor backgroundMainColor() const { return _backgroundMainColor; }
    inline void setBackgroundMainColor(const QColor &value) { _backgroundMainColor = value; }

    inline QColor borderColor() const { return _borderColor; }
    inline void setBorderColor(const QColor &value) { _borderColor = value; }

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
    QColor _borderColor;
    DriveSelectionWidget *_driveSelectionWidget;
    ProgressBarWidget *_progressBarWidget;
    StatusBarWidget *_statusBarWidget;
    QStackedWidget *_stackedWidget;
    QListWidget *_synchronizedListWidget;

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
    void onDriveSelected(int id);
    void onButtonBarToggled(int position);
    void onCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
};

}

