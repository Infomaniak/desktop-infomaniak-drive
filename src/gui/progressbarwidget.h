#pragma once

#include <QLabel>
#include <QProgressBar>
#include <QString>
#include <QWidget>

namespace KDC {

class ProgressBarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProgressBarWidget(QWidget *parent = nullptr);

    inline void setTransferTotalSize(unsigned long size) { _totalSize = size; }; // Ko
    void setTransferSize(unsigned long size); // Ko

private:
    unsigned long _totalSize; // Ko
    QProgressBar *_progressBar;
    QLabel *_progressLabel;

    QString sizeToString(unsigned long size);
};

}
