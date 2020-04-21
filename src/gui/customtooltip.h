#pragma once

#include <QWidget>

namespace KDC {

class CustomToolTip : public QWidget
{
    Q_OBJECT

public:
    CustomToolTip(const QString &text, QWidget *parent = nullptr);

private:
    QString _text;

    void paintEvent(QPaintEvent *event);
};

}
