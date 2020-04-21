#include "statusbarwidget.h"
#include "guiutility.h"
#include "common/utility.h"

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
                _pauseButton->setVisible(false);
                _resumeButton->setVisible(false);
                _syncButton->setVisible(false);
                break;
            case OCC::SyncResult::NotYetStarted:
                _statusLabel->setText(tr("Waiting..."));
                _pauseButton->setVisible(false);
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
                _syncButton->setVisible(true);
                break;
            case OCC::SyncResult::SyncPrepare:
                _statusLabel->setText(tr("Preparing to sync..."));
                _pauseButton->setVisible(true);
                _resumeButton->setVisible(false);
                _syncButton->setVisible(true);
                break;
            case OCC::SyncResult::Success:
                _statusLabel->setText(tr("You are up to date!"));
                _pauseButton->setVisible(true);
                _resumeButton->setVisible(false);
                _syncButton->setVisible(true);
                break;
            case OCC::SyncResult::Problem:
                _statusLabel->setText(tr("Some files haven't been synchronized."));
                _pauseButton->setVisible(false);
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

void StatusBarWidget::reset()
{
    setStatus(OCC::SyncResult::Undefined);
}

void StatusBarWidget::onPauseClicked()
{
    _pauseButton->setVisible(false);
    _resumeButton->setVisible(true);
    emit pauseSync();
}

void StatusBarWidget::onResumeClicked()
{
    _resumeButton->setVisible(false);
    _pauseButton->setVisible(true);
    emit resumeSync();
}

void StatusBarWidget::onSyncClicked()
{
    emit runSync();
}

}
