#include "customtoolbutton.h"
#include "guiutility.h"

#include <QApplication>
#include <QCursor>
#include <QEvent>
#include <QPainter>

namespace KDC {

static int defaultToolTipDuration = 3000; // ms

CustomToolButton::CustomToolButton(QWidget *parent)
    : QToolButton(parent)
    , _withMenu(false)
    , _baseIconSize(QSize())
    , _iconPath(QString())
    , _iconColor(QColor())
    , _iconColorHover(QColor())
    , _toolTipText(QString())
    , _toolTipDuration(defaultToolTipDuration)
    , _customToolTip(nullptr)
{
    connect(this, &CustomToolButton::baseIconSizeChanged, this, &CustomToolButton::onBaseIconSizeChanged);
    connect(this, &CustomToolButton::iconColorChanged, this, &CustomToolButton::onIconColorChanged);
}

void CustomToolButton::setWithMenu(bool withMenu)
{
    _withMenu = withMenu;
    applyIconSizeAndColor(_iconColor);
}

void CustomToolButton::onIconColorChanged()
{
    if (!_iconPath.isEmpty()) {
        applyIconSizeAndColor(_iconColor);
    }
}

bool CustomToolButton::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {
        if (!_toolTipText.isEmpty()) {
            if (!_customToolTip) {
                QRect widgetRect = geometry();
                QPoint position = parentWidget()->mapToGlobal((widgetRect.bottomLeft() + widgetRect.bottomRight()) / 2.0);
                _customToolTip = new CustomToolTip(_toolTipText, position, _toolTipDuration);
                _customToolTip->show();
                event->ignore();
                return true;
            }
        }
    }
    else if (event->type() == QEvent::Enter) {
        applyIconSizeAndColor(_iconColorHover);
    }
    else if (event->type() == QEvent::Leave
             || event->type() == QEvent::MouseButtonPress
             || event->type() == QEvent::MouseButtonDblClick) {
        applyIconSizeAndColor(_iconColor);
        if (_customToolTip) {
            emit _customToolTip->close();
            _customToolTip = nullptr;
        }
    }
    return QToolButton::event(event);
}

void CustomToolButton::applyIconSizeAndColor(const QColor &color)
{
    if (_baseIconSize != QSize()) {
        setIconSize(QSize(_withMenu ? 2 * _baseIconSize.width() : _baseIconSize.width(),
                          _baseIconSize.height()));
    }

    if (!_iconPath.isEmpty() && color.isValid()) {
        if (_withMenu) {
            setIcon(OCC::Utility::getIconMenuWithColor(_iconPath, color));
        }
        else {
            setIcon(OCC::Utility::getIconWithColor(_iconPath, color));
        }
    }
}

void CustomToolButton::onBaseIconSizeChanged()
{
    if (!_iconPath.isEmpty()) {
        applyIconSizeAndColor(_iconColor);
    }
}

}
