#include "rectwidget.h"

#include <QBrush>
#include <QPainter>
#include <QRect>

namespace KDC {

RectWidget::RectWidget(QWidget *parent)
    : QWidget(parent)
{
    _hboxLayout = new QHBoxLayout(this);
    setLayout(_hboxLayout);
}

void RectWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QRect widgetRect = rect();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen); // No border
    painter.fillRect(widgetRect, backgroundColor());
}

}
