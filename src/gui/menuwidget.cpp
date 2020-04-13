#include "menuwidget.h"
#include "guiutility.h"

#include <QAction>
#include <QGraphicsDropShadowEffect>
#include <QPainter>

namespace KDC {

const std::string MenuWidget::iconPathProperty = "iconPath";

static const int contentMargin = 10;
static const int cornerRadius = 5;
static const int shadowBlurRadius = 20;

MenuWidget::MenuWidget(QWidget *parent)
    : QMenu(parent)
    , _backgroundColor(QColor())
    , _iconColor(QColor())
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::X11BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground);

    setContentsMargins(contentMargin, contentMargin, contentMargin, contentMargin);

#ifdef Q_OS_WIN
    // Avoid a Qt bug => "UpdateLayeredWindowIndirect failed..." error and no hover
    setStyleSheet("margin: 10px");
#endif

    // Shadow
    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect(this);
    effect->setBlurRadius(shadowBlurRadius);
    effect->setOffset(0);
    setGraphicsEffect(effect);

    connect(this, &MenuWidget::iconColorChanged, this, &MenuWidget::onIconColorChanged);
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

void MenuWidget::onIconColorChanged()
{
    for (QAction *action : actions()) {
        QString iconPath = action->property(iconPathProperty.c_str()).toString();
        if (!iconPath.isEmpty()) {
            action->setIcon(OCC::Utility::getIconWithColor(iconPath, _iconColor));
        }
    }
}

}
