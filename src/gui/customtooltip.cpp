#include "customtooltip.h"

#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QStyleOption>
#include <QTimer>

namespace KDC {

static const int contentMargin = 10;
static const int cornerRadius = 5;
static const int shadowBlurRadius = 20;
static const int boxMargin = 10;
static const int offsetX = -30;
static const int offsetY = 10;

CustomToolTip::CustomToolTip(const QString &text, const QPoint &position, int toolTipDuration, QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::X11BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground);

    setContentsMargins(contentMargin, contentMargin, contentMargin, contentMargin);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(boxMargin, boxMargin, boxMargin, boxMargin);
    layout->setSpacing(0);

    QLabel *textLabel = new QLabel(this);
    textLabel->setObjectName("textLabel");
    textLabel->setText(text);
    layout->addWidget(textLabel);

    setLayout(layout);

    // Shadow
    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect(this);
    effect->setBlurRadius(shadowBlurRadius);
    effect->setOffset(0);
    setGraphicsEffect(effect);

    move(position + QPoint(offsetX, offsetY));
    if (toolTipDuration) {
        QTimer::singleShot(toolTipDuration, this, &CustomToolTip::close);
    }
}

void CustomToolTip::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QRect intRect = rect().marginsRemoved(QMargins(contentMargin, contentMargin, contentMargin - 1, contentMargin - 1));

    QPainterPath painterPath;
    painterPath.addRoundedRect(intRect, cornerRadius, cornerRadius);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(QColor("#FFFFFF"));
    painter.setPen(Qt::NoPen);
    painter.drawPath(painterPath);
}

}
