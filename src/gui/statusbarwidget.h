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

#include "halfroundrectwidget.h"
#include "customtoolbutton.h"
#include "syncresult.h"
#include "theme.h"

#include <QLabel>
#include <QWidget>

namespace KDC {

class StatusBarWidget : public HalfRoundRectWidget
{
    Q_OBJECT

public:
    explicit StatusBarWidget(QWidget *parent = nullptr);

    void setStatus(bool paused, OCC::SyncResult::Status status, qint64 currentFile = 0,
                   qint64 totalFiles = 0, qint64 estimatedRemainingTime = 0);
    void setSeveralDrives(bool severalDrives);
    void reset();

signals:
    void pauseSync(bool all);
    void resumeSync(bool all);
    void runSync(bool all);

private:
    OCC::SyncResult::Status _status;
    bool _severalDrives;
    QLabel *_statusIconLabel;
    QLabel *_statusLabel;
    CustomToolButton *_pauseButton;
    CustomToolButton *_resumeButton;
    CustomToolButton *_syncButton;

private slots:
    void onPauseClicked();
    void onResumeClicked();
    void onSyncClicked();
    void onPauseSync();
    void onPauseAllSync();
    void onResumeSync();
    void onResumeAllSync();
    void onRunSync();
    void onRunAllSync();
};

}

