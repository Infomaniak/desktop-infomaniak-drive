#pragma once

#include <QDialog>

namespace KDC {

class CustomToolTip : public QDialog
{
    Q_OBJECT

public:
    CustomToolTip(const QString &text, const QPoint &position, int toolTipDuration, QWidget *parent = nullptr);

private:
    void paintEvent(QPaintEvent *event) override;
};

}
