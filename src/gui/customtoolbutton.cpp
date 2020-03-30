#include "customtoolbutton.h"
#include "guiutility.h"

namespace KDC {

CustomToolButton::CustomToolButton(QWidget *parent)
    : QToolButton(parent)
    , _iconPath(QString())
    , _iconColor(Qt::black)
{
    connect(this, &CustomToolButton::iconColorChanged, this, &CustomToolButton::onIconColorChanged);
}

void CustomToolButton::onIconColorChanged()
{
    if (!_iconPath.isEmpty()) {
        setIcon(OCC::Utility::getPixmapWithColor(_iconPath, _iconColor));
    }
}

}
