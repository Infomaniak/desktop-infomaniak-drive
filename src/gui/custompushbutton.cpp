#include "custompushbutton.h"
#include "guiutility.h"

namespace KDC {

CustomPushButton::CustomPushButton(QWidget *parent)
    : QPushButton(parent)
    , _iconPath(QString())
    , _iconColor(QColor())
    , _iconColorChecked(QColor())
{
    setFlat(true);
    setCheckable(true);

    connect(this, &CustomPushButton::iconColorChanged, this, &CustomPushButton::onIconColorChanged);
    connect(this, &CustomPushButton::iconColorCheckedChanged, this, &CustomPushButton::onIconColorCheckedChanged);
    connect(this, &CustomPushButton::toggled, this, &CustomPushButton::onToggle);
}

CustomPushButton::CustomPushButton(const QString &text, QWidget *parent)
    : CustomPushButton(parent)
{
    setText(text);
}

void CustomPushButton::onIconColorChanged()
{
    onToggle(isChecked());
}

void CustomPushButton::onIconColorCheckedChanged()
{
    onToggle(isChecked());
}

void CustomPushButton::onToggle(bool checked)
{
    if (!_iconPath.isEmpty()) {
        setIcon(OCC::Utility::getIconWithColor(_iconPath, checked ? _iconColorChecked : _iconColor));
    }
}

}
