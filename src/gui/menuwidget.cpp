#include "menuwidget.h"
#include "guiutility.h"

#include <QAction>
#include <QGraphicsDropShadowEffect>
#include <QPainter>

namespace KDC {

const std::string MenuWidget::actionTypeProperty = "actionType";

static const int contentMargin = 10;
static const int cornerRadius = 5;
static const int shadowBlurRadius = 10;
static const int offsetX = -30;
static const int offsetY = 10;

MenuWidget::MenuWidget(QWidget *parent)
    : QMenu(parent)
    , _backgroundColor(QColor())
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

MenuWidget::MenuWidget(const QString &title, QWidget *parent)
    : MenuWidget(parent)
{
    setTitle(title);
}

QAction *MenuWidget::exec(const QPoint &pos, bool offsetAuto)
{
    if (offsetAuto) {
        return QMenu::exec(pos + QPoint(offsetX, offsetY));
    }
    else {
        return QMenu::exec(pos);
    }
}

void MenuWidget::paintEvent(QPaintEvent *event)
{
    QRect intRect = rect().marginsRemoved(QMargins(contentMargin, contentMargin, contentMargin - 1, contentMargin - 1));

    QPainterPath painterPath;
    painterPath.addRoundedRect(intRect, cornerRadius, cornerRadius);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(backgroundColor());
    painter.setPen(Qt::NoPen);
    painter.drawPath(painterPath);

    QMenu::paintEvent(event);
}

}
