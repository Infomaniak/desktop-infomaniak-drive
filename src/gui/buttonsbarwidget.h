#pragma once

#include "custompushbutton.h"

#include <QColor>
#include <QBoxLayout>
#include <QList>
#include <QPaintEvent>
#include <QWidget>

namespace KDC {

class ButtonsBarWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor background_color READ backgroundColor WRITE setBackgroundColor)

public:
    explicit ButtonsBarWidget(QWidget *parent = nullptr);

    inline QColor backgroundColor() const { return _backgroundColor; }
    inline void setBackgroundColor(const QColor& value) { _backgroundColor = value; }

    void insertButton(int position, CustomPushButton *button);

signals:
    void buttonToggled(int position);

private:
    QColor _backgroundColor;
    QHBoxLayout *_hboxLayout;
    QList<CustomPushButton*> buttonsList;

    void paintEvent(QPaintEvent *event) override;

private slots:
    void onButtonToggled(bool checked);
};

}

