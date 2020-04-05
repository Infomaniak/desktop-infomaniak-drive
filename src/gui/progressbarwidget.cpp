#include "progressbarwidget.h"

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

void ProgressBarWidget::setTransferSize(unsigned long size)
{
    if (size >= 0 && size <= _totalSize) {
        int pct = (_totalSize > 0 ? size * 100.0 / _totalSize : 100);
        _progressBar->setValue(pct);
        _progressLabel->setText(sizeToString(size) + " / " + sizeToString(_totalSize));
    }
}

QString ProgressBarWidget::sizeToString(unsigned long size)
{
    QString sizeStr;
    if (size < 1000) {
        sizeStr = QString("%1 Ko").arg(size);
    }
    else if (size < 1000000) {
        sizeStr = QString("%1 Mo").arg(int(size / 1000));
    }
    else {
        sizeStr = QString("%1 To").arg(int(size / 1000000));
    }

    return sizeStr;
}

}
