#include "customtooltip.h"

#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QStyleOption>

namespace KDC {

static const int contentMargin = 10;
static const int cornerRadius = 5;
static const int shadowBlurRadius = 20;

CustomToolTip::CustomToolTip(const QString &text, QWidget *parent)
    : QWidget(parent)
    , _text(text)
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::X11BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground);

    setContentsMargins(contentMargin, contentMargin, contentMargin, contentMargin);

    // Shadow
    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect(this);
    effect->setBlurRadius(shadowBlurRadius);
    effect->setOffset(0);
    setGraphicsEffect(effect);
}

void CustomToolTip::paintEvent(QPaintEvent *event)
{
    QRect intRect = rect().marginsRemoved(QMargins(contentMargin, contentMargin, contentMargin - 1, contentMargin - 1));

    QPainterPath painterPath;
    painterPath.addRoundedRect(intRect, cornerRadius, cornerRadius);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    //painter.setBrush(backgroundColor());
    painter.setPen(Qt::NoPen);
    painter.drawPath(painterPath);

}

}
