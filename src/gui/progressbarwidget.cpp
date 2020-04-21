#include "progressbarwidget.h"
#include "common/utility.h"

#include <QHBoxLayout>

namespace KDC {

static const int hMargin = 15;
static const int vMargin = 0;
static const int hSpacing = 15;
static const int progressBarMin = 0;
static const int progressBarMax = 100;

ProgressBarWidget::ProgressBarWidget(QWidget *parent)
    : QWidget(parent)
    , _totalSize(0)
    , _progressBar(nullptr)
    , _progressLabel(nullptr)
{
    QHBoxLayout *hboxProgressBar = new QHBoxLayout(this);
    hboxProgressBar->setContentsMargins(hMargin, vMargin, hMargin, vMargin);
    hboxProgressBar->setSpacing(hSpacing);

    _progressBar = new QProgressBar(this);
    _progressBar->setMinimum(progressBarMin);
    _progressBar->setMaximum(progressBarMax);
    _progressBar->setFormat(QString());
    hboxProgressBar->addWidget(_progressBar);

    _progressLabel = new QLabel(this);
    _progressLabel->setObjectName("progressLabel");
    hboxProgressBar->addWidget(_progressLabel);
}

void ProgressBarWidget::setUsedSize(qint64 totalSize, qint64 size)
{
    _totalSize = totalSize;
    if (_totalSize > 0) {
        int pct = size <= _totalSize ? qRound(double(size) / double(_totalSize) * 100.0) : 100;
        _progressBar->setValue(pct);
        _progressLabel->setText(OCC::Utility::octetsToString(size) + " / " + OCC::Utility::octetsToString(_totalSize));
    }
    else {
        // -1 => not computed; -2 => unknown; -3 => unlimited
        if (_totalSize == 0 || _totalSize == -1) {
            _progressBar->setValue(0);
            _progressLabel->setText(tr("No storage usage information available"));
        } else {
            _progressBar->setValue(0);
            _progressLabel->setText(tr("%1 in use").arg(OCC::Utility::octetsToString(size)));
        }
    }
}

void ProgressBarWidget::reset()
{
    setUsedSize(0, 0);
}

}
