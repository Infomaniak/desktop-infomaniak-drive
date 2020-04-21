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

    void setUsedSize(qint64 totalSize, qint64 size);
    void reset();

private:
    qint64 _totalSize; // Ko
    QProgressBar *_progressBar;
    QLabel *_progressLabel;
};

}
