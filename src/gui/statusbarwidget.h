#pragma once

#include "halfroundrectwidget.h"
#include "customtoolbutton.h"
#include "syncresult.h"
#include "theme.h"

#include <QLabel>
#include <QTime>
#include <QWidget>

namespace KDC {

class StatusBarWidget : public HalfRoundRectWidget
{
    Q_OBJECT

public:
    explicit StatusBarWidget(QWidget *parent = nullptr);

    void setStatus(OCC::SyncResult::Status status, qint64 currentFile = 0, qint64 totalFiles = 0,
                   qint64 estimatedRemainingTime = 0);
    void reset();

signals:
    void pauseSync();
    void resumeSync();
    void runSync();

private:
    OCC::SyncResult::Status _status;
    QLabel *_statusIconLabel;
    QLabel *_statusLabel;
    CustomToolButton *_pauseButton;
    CustomToolButton *_resumeButton;
    CustomToolButton *_syncButton;

private slots:
    void onPauseClicked();
    void onResumeClicked();
    void onSyncClicked();
};

}

