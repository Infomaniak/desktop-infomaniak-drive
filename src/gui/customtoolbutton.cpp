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
    , _defaultIconSize(QSize())
    , _setIconSize(false)
    , _iconPath(QString())
    , _iconColor(QColor())
    , _iconColorHover(QColor())
    , _toolTipText(QString())
    , _toolTipDuration(defaultToolTipDuration)
    , _customToolTip(nullptr)
{
    connect(this, &CustomToolButton::iconColorChanged, this, &CustomToolButton::onIconColorChanged);
}

void CustomToolButton::setWithMenu(bool withMenu)
{
    _withMenu = withMenu;
    _setIconSize = true;
}

void CustomToolButton::onIconColorChanged()
{
    if (!_iconPath.isEmpty()) {
        applyIconColor(_iconColor);
    }
}

void CustomToolButton::paintEvent(QPaintEvent *event)
{
    if (_defaultIconSize == QSize()) {
        _defaultIconSize = iconSize();
    }
    /*if (_setIconSize) {
        _setIconSize = false;
        setIconSize(QSize(_withMenu ? 2 * _defaultIconSize.width() : _defaultIconSize.width(),
                          _defaultIconSize.height()));
        // Force redraw
        applyIconColor(_iconColor);
        return;
    }*/
    QToolButton::paintEvent(event);
}

bool CustomToolButton::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {
        if (!_toolTipText.isEmpty()) {
            if (!_customToolTip) {
                _customToolTip = new CustomToolTip(_toolTipText, QCursor::pos(), _toolTipDuration);
                _customToolTip->show();
                event->ignore();
                return true;
            }
        }
    }
    else if (event->type() == QEvent::Enter) {
        applyIconColor(_iconColorHover);
    }
    else if (event->type() == QEvent::Leave
             || event->type() == QEvent::MouseButtonPress
             || event->type() == QEvent::MouseButtonDblClick) {
        applyIconColor(_iconColor);
        if (_customToolTip) {
            emit _customToolTip->close();
            _customToolTip = nullptr;
        }
    }
    return QToolButton::event(event);
}

void CustomToolButton::applyIconColor(const QColor &color)
{
    if (!_iconPath.isEmpty() && color.isValid()) {
        if (_defaultIconSize != QSize()) {
            setIconSize(QSize(_withMenu ? 2 * _defaultIconSize.width() : _defaultIconSize.width(),
                              _defaultIconSize.height()));
        }
        if (_withMenu) {
            setIcon(OCC::Utility::getIconMenuWithColor(_iconPath, color));
        }
        else {
            setIcon(OCC::Utility::getIconWithColor(_iconPath, color));
        }
    }
}

}
