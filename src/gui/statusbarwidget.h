#pragma once

#include "halfroundrectwidget.h"
#include "syncresult.h"

#include <QLabel>
#include <QTime>
#include <QWidget>

namespace KDC {

class StatusBarWidget : public HalfRoundRectWidget
{
    Q_OBJECT

public:
    enum transferStatus {
        InProgress = 0,
        Paused
    };

    explicit StatusBarWidget(QWidget *parent = nullptr);

    void setStatus(OCC::SyncResult::Status status, int fileNum, int fileCount, const QTime &time);

signals:
    void statusChanged(transferStatus status);

private:
    QLabel *_statusIconLabel;
    QLabel *_statusLabel;
    transferStatus _status;

private slots:
    void onStatusClicked();
};

}

