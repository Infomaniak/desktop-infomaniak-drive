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

void StatusBarWidget::setStatus(OCC::SyncResult::Status status, qint64 currentFile, qint64 totalFiles,
                                qint64 estimatedRemainingTime)
{
    if (_statusIconLabel) {
        _statusIconLabel->setPixmap(OCC::Theme::instance()->syncStateIcon(status).pixmap(QSize(statusIconSize, statusIconSize)));
        _statusIconLabel->setVisible(true);
        if (_statusLabel) {
            QString timeStr;
            switch (status) {
            case OCC::SyncResult::Undefined:
                // this can happen if no sync connections are configured.
                _statusLabel->setText(tr("No drive configured."));
                _pauseButton->setVisible(true);
                _resumeButton->setVisible(false);
                _syncButton->setVisible(false);
                break;
            case OCC::SyncResult::NotYetStarted:
                _statusLabel->setText(tr("Waiting..."));
                _pauseButton->setVisible(true);
                _resumeButton->setVisible(false);
                _syncButton->setVisible(false);
                break;
            case OCC::SyncResult::SyncRunning:
                _statusLabel->setText(tr("Synchronization in progress (%1 on %2)\n%3 left...")
                                      .arg(currentFile).arg(totalFiles)
                                      .arg(OCC::Utility::durationToDescriptiveString1(estimatedRemainingTime)));
                _pauseButton->setVisible(true);
                _resumeButton->setVisible(false);
                _syncButton->setVisible(false);
                break;
            case OCC::SyncResult::SyncAbortRequested:
            case OCC::SyncResult::Paused:
                _statusLabel->setText(tr("Synchronization paused."));
                _pauseButton->setVisible(false);
                _resumeButton->setVisible(true);
                _syncButton->setVisible(false);
                break;
            case OCC::SyncResult::SyncPrepare:
                _statusLabel->setText(tr("Preparing to sync..."));
                _pauseButton->setVisible(true);
                _resumeButton->setVisible(false);
                _syncButton->setVisible(false);
                break;
            case OCC::SyncResult::Success:
                _statusLabel->setText(tr("You are up to date!"));
                _pauseButton->setVisible(true);
                _resumeButton->setVisible(false);
                _syncButton->setVisible(true);
                break;
            case OCC::SyncResult::Problem:
                _statusLabel->setText(tr("Some files haven't been synchronized."));
                _pauseButton->setVisible(true);
                _resumeButton->setVisible(false);
                _syncButton->setVisible(true);
                break;
            case OCC::SyncResult::Error:
            case OCC::SyncResult::SetupError:
            // FIXME: Use state-problem once we have an icon.
            default:
                _pauseButton->setVisible(false);
                _resumeButton->setVisible(false);
                _syncButton->setVisible(false);
            }
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
    setStatus(OCC::SyncResult::Undefined);
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
        _pauseButton->setVisible(false);
        _resumeButton->setVisible(false);
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
