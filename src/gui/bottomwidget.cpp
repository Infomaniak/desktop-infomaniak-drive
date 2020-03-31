#include "bottomwidget.h"

#include <QPainter>

namespace KDC {

BottomWidget::BottomWidget(QWidget *parent)
    : QWidget(parent)
{
}

void BottomWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPointF rectangleLeftTop(0, 0);
    QPointF rectangleRightTop(rect().width(), 0);

    QPainterPath painterPath(rectangleLeftTop);
    painterPath.arcTo(QRect(0, - rect().height(), 2 * rect().height(), 2 * rect().height()), 180, 90);
    painterPath.arcTo(QRect(rect().width() - 2 * rect().height(), - rect().height(), 2 * rect().height(), 2 * rect().height()), 270, 90);
    painterPath.lineTo(rectangleRightTop);
    painterPath.closeSubpath();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(backgroundColor());
    painter.setPen(Qt::NoPen);
    painter.drawPath(painterPath);
}

}
