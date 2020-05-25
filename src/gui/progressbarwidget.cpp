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
    QHBoxLayout *hboxProgressBar = new QHBoxLayout();
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
    _progressBar->setVisible(true);
    _progressLabel->setVisible(true);
    if (_totalSize > 0) {
        int pct = size <= _totalSize ? qRound(double(size) / double(_totalSize) * 100.0) : 100;
        _progressBar->setValue(pct);
        _progressLabel->setText(OCC::Utility::octetsToString(size) + " / " + OCC::Utility::octetsToString(_totalSize));
    }
    else {
        // -1 => not computed; -2 => unknown; -3 => unlimited
        if (_totalSize == 0 || _totalSize == -1) {
            _progressBar->setValue(0);
            _progressLabel->setText(QString());
        } else {
            _progressBar->setValue(0);
            _progressLabel->setText(tr("%1 in use").arg(OCC::Utility::octetsToString(size)));
        }
    }
}

void ProgressBarWidget::reset()
{
    _totalSize = 0;
    _progressBar->setVisible(false);
    _progressLabel->setVisible(false);
}

}
