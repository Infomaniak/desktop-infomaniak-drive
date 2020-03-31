#include "halfroundrectwidget.h"

#include <QBrush>
#include <QPainter>
#include <QPainterPath>
#include <QPoint>
#include <QRect>

namespace KDC {

static const int cornerRadius = 40;

HalfRoundRectWidget::HalfRoundRectWidget(QWidget *parent)
    : QWidget(parent)
{
    _hboxLayout = new QHBoxLayout(this);
    setLayout(_hboxLayout);
}

void HalfRoundRectWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    // Half round rectangle
    QPointF rectangleLeftBottom(0, rect().height());
    QPointF rectangleRightBottom(rect().width(), rect().height());

    QPainterPath painterPath1(rectangleLeftBottom);
    painterPath1.arcTo(QRect(0, rect().height() - cornerRadius, cornerRadius, cornerRadius), 180, 90);
    painterPath1.moveTo(rectangleLeftBottom);

    QPainterPath painterPath2(rectangleRightBottom);
    painterPath2.arcTo(QRect(rect().width() - cornerRadius, rect().height() - cornerRadius, cornerRadius, cornerRadius), 270, 90);
    painterPath2.moveTo(rectangleRightBottom);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(bottomCornersColor());
    painter.setPen(Qt::NoPen);
    painter.drawPath(painterPath1);
    painter.drawPath(painterPath2);
}

}
