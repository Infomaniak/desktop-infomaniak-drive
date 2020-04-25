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

#include "statusbarwidget.h"
#include "menuitemwidget.h"
#include "menuwidget.h"
#include "guiutility.h"
#include "common/utility.h"

#include <QWidgetAction>

namespace KDC {

static const int hMargin = 15;
static const int vMargin = 15;
static const int statusIconSize = 24;

StatusBarWidget::StatusBarWidget(QWidget *parent)
    : HalfRoundRectWidget(parent)
    , _status(OCC::SyncResult::Undefined)
    , _statusIconLabel(nullptr)
    , _statusLabel(nullptr)
    , _pauseButton(nullptr)
    , _resumeButton(nullptr)
    , _syncButton(nullptr)
{
    setContentsMargins(hMargin, vMargin, hMargin, vMargin);

    _statusIconLabel = new QLabel(this);
    _statusIconLabel->setVisible(false);
    addWidget(_statusIconLabel);

    _statusLabel = new QLabel(this);
    _statusLabel->setObjectName("statusLabel");
    addWidget(_statusLabel);
    addStretch();

    _pauseButton = new CustomToolButton(this);
    _pauseButton->setIconPath(":/client/resources/icons/actions/pause.svg");
    _pauseButton->setToolTip(tr("Pause synchronization"));
    _pauseButton->setVisible(false);
    addWidget(_pauseButton);

    _resumeButton = new CustomToolButton(this);
    _resumeButton->setIconPath(":/client/resources/icons/actions/start.svg");
    _resumeButton->setToolTip(tr("Resume synchronization"));
    _resumeButton->setVisible(false);
    addWidget(_resumeButton);

    _syncButton = new CustomToolButton(this);
    _syncButton->setIconPath(":/client/resources/icons/actions/sync-circle.svg");
    _syncButton->setToolTip(tr("Force synchronization"));
    _syncButton->setVisible(false);
    addWidget(_syncButton);

    connect(_pauseButton, &CustomToolButton::clicked, this, &StatusBarWidget::onPauseClicked);
    connect(_resumeButton, &CustomToolButton::clicked, this, &StatusBarWidget::onResumeClicked);
    connect(_syncButton, &CustomToolButton::clicked, this, &StatusBarWidget::onSyncClicked);
}

void StatusBarWidget::setStatus(bool paused, OCC::SyncResult::Status status, qint64 currentFile,
                                qint64 totalFiles, qint64 estimatedRemainingTime)
{
    if (paused || status == OCC::SyncResult::Paused
            || status == OCC::SyncResult::SyncAbortRequested) {
        _statusIconLabel->setPixmap(OCC::Theme::instance()->syncStateIcon(OCC::SyncResult::Paused)
                                    .pixmap(QSize(statusIconSize, statusIconSize)));
        _statusLabel->setText(tr("Synchronization paused."));
        _statusIconLabel->setVisible(true);
        _pauseButton->setVisible(false);
        _resumeButton->setVisible(true);
        _syncButton->setVisible(false);
    }
    else if (totalFiles > 0) {
        _statusIconLabel->setPixmap(OCC::Theme::instance()->syncStateIcon(OCC::SyncResult::SyncRunning)
                                    .pixmap(QSize(statusIconSize, statusIconSize)));
        _statusLabel->setText(tr("Synchronization in progress (%1 on %2)\n%3 left...")
                              .arg(currentFile).arg(totalFiles)
                              .arg(OCC::Utility::durationToDescriptiveString1(estimatedRemainingTime)));
        _statusIconLabel->setVisible(true);
        _pauseButton->setVisible(true);
        _resumeButton->setVisible(false);
        _syncButton->setVisible(true);
    }
    else {
        _statusIconLabel->setPixmap(OCC::Theme::instance()->syncStateIcon(status)
                                    .pixmap(QSize(statusIconSize, statusIconSize)));

        QString timeStr;
        switch (status) {
        case OCC::SyncResult::Undefined:
            _statusLabel->setText(tr("No folder to synchronize."));
            _statusIconLabel->setVisible(true);
            _pauseButton->setVisible(false);
            _resumeButton->setVisible(false);
            _syncButton->setVisible(false);
            break;
        case OCC::SyncResult::NotYetStarted:
            _statusLabel->setText(tr("Waiting for synchronization..."));
            _statusIconLabel->setVisible(true);
            _pauseButton->setVisible(true);
            _resumeButton->setVisible(false);
            _syncButton->setVisible(true);
            break;
        case OCC::SyncResult::SyncPrepare:
            _statusLabel->setText(tr("Preparing to synchronize..."));
            _statusIconLabel->setVisible(true);
            _pauseButton->setVisible(true);
            _resumeButton->setVisible(false);
            _syncButton->setVisible(true);
            break;
        case OCC::SyncResult::Success:
            _statusLabel->setText(tr("You are up to date!"));
            _statusIconLabel->setVisible(true);
            _pauseButton->setVisible(true);
            _resumeButton->setVisible(false);
            _syncButton->setVisible(true);
            break;
        case OCC::SyncResult::Problem:
            _statusLabel->setText(tr("Some files haven't been synchronized."));
            _statusIconLabel->setVisible(true);
            _pauseButton->setVisible(false);
            _resumeButton->setVisible(false);
            _syncButton->setVisible(true);
            break;
        case OCC::SyncResult::Error:
            _statusLabel->setText(tr("Synchronization error."));
            _statusIconLabel->setVisible(true);
            _pauseButton->setVisible(false);
            _resumeButton->setVisible(false);
            _syncButton->setVisible(true);
            break;
        case OCC::SyncResult::SetupError:
            _statusLabel->setText(tr("Setup error."));
            _statusIconLabel->setVisible(true);
            _pauseButton->setVisible(false);
            _resumeButton->setVisible(false);
            _syncButton->setVisible(true);
            break;
        default:
            // Should not happen
            _statusLabel->setText("");
            _statusIconLabel->setVisible(false);
            _pauseButton->setVisible(false);
            _resumeButton->setVisible(false);
            _syncButton->setVisible(false);
        }
    }
}

void StatusBarWidget::setSeveralDrives(bool severalDrives)
{
    _severalDrives = severalDrives;
    _pauseButton->setWithMenu(_severalDrives);
    _resumeButton->setWithMenu(_severalDrives);
    _syncButton->setWithMenu(_severalDrives);
}

void StatusBarWidget::reset()
{
    setStatus(false, OCC::SyncResult::Undefined);
}

void StatusBarWidget::onPauseClicked()
{
    bool resetButtons = false;

    if (_severalDrives) {
        MenuWidget *menu = new MenuWidget(this);

        // Pause
        QWidgetAction *pauseAction = new QWidgetAction(this);
        MenuItemWidget *pauseMenuItemWidget = new MenuItemWidget(tr("Pause synchronization"));
        pauseMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/pause.svg");
        pauseAction->setDefaultWidget(pauseMenuItemWidget);
        connect(pauseAction, &QWidgetAction::triggered, this, &StatusBarWidget::onPauseSync);
        menu->addAction(pauseAction);

        // Pause all
        QWidgetAction *pauseAllAction = new QWidgetAction(this);
        MenuItemWidget *pauseAllMenuItemWidget = new MenuItemWidget(tr("Pause synchronization for all kDrives"));
        pauseAllMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/pause.svg");
        pauseAllAction->setDefaultWidget(pauseAllMenuItemWidget);
        connect(pauseAllAction, &QWidgetAction::triggered, this, &StatusBarWidget::onPauseAllSync);
        menu->addAction(pauseAllAction);

        if (menu->exec(QWidget::mapToGlobal(_pauseButton->geometry().center()), true)) {
            resetButtons = true;
        }
    }
    else {
        emit pauseSync(false);
        resetButtons = true;
    }

    if (resetButtons) {
        // Status is NOT updated after a sync pause
        _pauseButton->setVisible(false);
        _resumeButton->setVisible(true);
        _syncButton->setVisible(false);
    }
}

void StatusBarWidget::onResumeClicked()
{
    bool resetButtons = false;

    if (_severalDrives) {
        MenuWidget *menu = new MenuWidget(this);

        // Resume
        QWidgetAction *resumeAction = new QWidgetAction(this);
        MenuItemWidget *resumeMenuItemWidget = new MenuItemWidget(tr("Resume synchronization"));
        resumeMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/resume.svg");
        resumeAction->setDefaultWidget(resumeMenuItemWidget);
        connect(resumeAction, &QWidgetAction::triggered, this, &StatusBarWidget::onResumeSync);
        menu->addAction(resumeAction);

        // Resume all
        QWidgetAction *resumeAllAction = new QWidgetAction(this);
        MenuItemWidget *resumeAllMenuItemWidget = new MenuItemWidget(tr("Resume synchronization for all kDrives"));
        resumeAllMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/resume.svg");
        resumeAllAction->setDefaultWidget(resumeAllMenuItemWidget);
        connect(resumeAllAction, &QWidgetAction::triggered, this, &StatusBarWidget::onResumeAllSync);
        menu->addAction(resumeAllAction);

        if (menu->exec(QWidget::mapToGlobal(_resumeButton->geometry().center()), true)) {
            resetButtons = true;
        }
    }
    else {
        emit resumeSync(false);
        resetButtons = true;
    }

    if (resetButtons) {
        // Status is updated after a sync resume
        _pauseButton->setVisible(false);
        _resumeButton->setVisible(false);
        _syncButton->setVisible(false);
    }
}

void StatusBarWidget::onSyncClicked()
{
    bool resetButtons = false;

    if (_severalDrives) {
        MenuWidget *menu = new MenuWidget(this);

        // Force sync
        QWidgetAction *runAction = new QWidgetAction(this);
        MenuItemWidget *runMenuItemWidget = new MenuItemWidget(tr("Force synchronization"));
        runMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/sync-circle.svg");
        runAction->setDefaultWidget(runMenuItemWidget);
        connect(runAction, &QWidgetAction::triggered, this, &StatusBarWidget::onRunSync);
        menu->addAction(runAction);

        // Force sync all
        QWidgetAction *runAllAction = new QWidgetAction(this);
        MenuItemWidget *runAllMenuItemWidget = new MenuItemWidget(tr("Force synchronization for all kDrives"));
        runAllMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/sync-circle.svg");
        runAllAction->setDefaultWidget(runAllMenuItemWidget);
        connect(runAllAction, &QWidgetAction::triggered, this, &StatusBarWidget::onRunAllSync);
        menu->addAction(runAllAction);

        if (menu->exec(QWidget::mapToGlobal(_syncButton->geometry().center()), true)) {
            resetButtons = true;
        }
    }
    else {
        emit runSync(false);
        resetButtons = true;
    }

    if (resetButtons) {
        // Status is updated after a sync run
        _pauseButton->setVisible(false);
        _resumeButton->setVisible(false);
        _syncButton->setVisible(false);
    }
}

void StatusBarWidget::onPauseSync()
{
    emit pauseSync(false);
}

void StatusBarWidget::onPauseAllSync()
{
    emit pauseSync(true);
}

void StatusBarWidget::onResumeSync()
{
    emit resumeSync(false);
}

void StatusBarWidget::onResumeAllSync()
{
    emit resumeSync(true);
}

void StatusBarWidget::onRunSync()
{
    emit runSync(false);
}

void StatusBarWidget::onRunAllSync()
{
    emit runSync(true);
}

}
