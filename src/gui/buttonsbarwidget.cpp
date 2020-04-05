#include "buttonsbarwidget.h"

#include <QBrush>
#include <QPainter>
#include <QRect>

namespace KDC {

static const int hMargin = 30;
static const int vMargin = 20;

ButtonsBarWidget::ButtonsBarWidget(QWidget *parent)
    : QWidget(parent)
    , _backgroundColor(QColor())
    , _hboxLayout(nullptr)
{
    _hboxLayout = new QHBoxLayout(this);
    _hboxLayout->setContentsMargins(hMargin, vMargin, hMargin, vMargin);
    setLayout(_hboxLayout);
}

void ButtonsBarWidget::insertButton(int position, CustomPushButton *button)
{
    _hboxLayout->insertWidget(position, button);
    connect(button, &CustomPushButton::toggled, this, &ButtonsBarWidget::onButtonToggled);
    buttonsList.insert(position, button);
    if (buttonsList.size() == 1) {
        button->setChecked(true);
    }
}

void ButtonsBarWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.fillRect(rect(), backgroundColor());
}

void ButtonsBarWidget::onButtonToggled(bool checked)
{
    if (checked) {
        int position = 0;
        for (auto btn : buttonsList) {
            if (btn != sender()) {
                btn->setChecked(false);
            }
            else {
                emit buttonToggled(position);
            }
            position++;
        }
    }
}

}
