#include "statusbarwidget.h"
#include "customtoolbutton.h"
#include "guiutility.h"
#include "theme.h"

namespace KDC {

static const int hMargin = 15;
static const int vMargin = 15;
static const int statusIconSize = 24;

StatusBarWidget::StatusBarWidget(QWidget *parent)
    : HalfRoundRectWidget(parent)
    , _statusIconLabel(nullptr)
    , _statusLabel(nullptr)
    , _status(transferStatus::InProgress)
{
    setContentsMargins(hMargin, vMargin, hMargin, vMargin);

    _statusIconLabel = new QLabel(this);
    addWidget(_statusIconLabel);

    _statusLabel = new QLabel(this);
    addWidget(_statusLabel);
    addStretch();

    CustomToolButton *pauseButton = new CustomToolButton(this);
    pauseButton->setIconPath(":/client/resources/icons/actions/pause.svg");
    addWidget(pauseButton);

    connect(pauseButton, &CustomToolButton::clicked, this, &StatusBarWidget::onStatusClicked);
}

void StatusBarWidget::setStatus(OCC::SyncResult::Status status, int fileNum, int fileCount, const QTime &time)
{
    if (_statusIconLabel) {
        _statusIconLabel->setPixmap(OCC::Theme::instance()->syncStateIcon(status).pixmap(QSize(statusIconSize, statusIconSize)));
        QString timeStr;
        timeStr += (time.hour() > 0 ? QString(tr("%1 hour(s) and ")).arg(time.hour()) : "");
        timeStr += QString(tr("%1 minute(s)")).arg(time.minute());
        _statusLabel->setText(tr("Synchronisation en cours (%1 sur %2)\n%3 restantes...")
                              .arg(fileNum).arg(fileCount).arg(timeStr));
    }
}

void StatusBarWidget::onStatusClicked()
{
    _status = (_status == transferStatus::InProgress ? transferStatus::Paused : transferStatus::InProgress);
    emit statusChanged(_status);
}

}
