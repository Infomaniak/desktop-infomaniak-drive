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
#include "accountinfo.h"
#include "syncresult.h"
#include "theme.h"

#include <QLabel>
#include <QWidget>

namespace KDC {

class StatusBarWidget : public HalfRoundRectWidget
{
    Q_OBJECT

public:
    enum ActionType {
        Drive = 0,
        Folder,
        AllDrives
    };

    explicit StatusBarWidget(QWidget *parent = nullptr);

    void setStatus(bool paused, bool unresolvedConflicts, OCC::SyncResult::Status status,
                   qint64 currentFile = 0, qint64 totalFiles = 0, qint64 estimatedRemainingTime = 0);
    void setCurrentAccount(const AccountInfo *accountInfo);
    void setSeveralDrives(bool severalDrives);
    void reset();

signals:
    void pauseSync(ActionType type, const QString &id = QString());
    void resumeSync(ActionType type, const QString &id = QString());
    void runSync(ActionType type, const QString &id = QString());
    void linkActivated(const QString &link);

private:
    OCC::SyncResult::Status _status;
    const AccountInfo *_accountInfo;
    bool _severalDrives;
    QLabel *_statusIconLabel;
    QLabel *_statusLabel;
    CustomToolButton *_pauseButton;
    CustomToolButton *_resumeButton;
    CustomToolButton *_syncButton;

    void onLinkActivated(const QString &link);

private slots:
    void onPauseClicked();
    void onResumeClicked();
    void onSyncClicked();
    void onPauseSync();
    void onPauseFolderSync();
    void onPauseAllSync();
    void onResumeSync();
    void onResumeFolderSync();
    void onResumeAllSync();
    void onRunSync();
    void onRunFolderSync();
    void onRunAllSync();
};

}

