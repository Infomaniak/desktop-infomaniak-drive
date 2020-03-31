#pragma once

#include <QColor>
#include <QPaintEvent>
#include <QWidget>

namespace KDC {

class BottomWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor background_color READ backgroundColor WRITE setBackgroundColor)

public:
    explicit BottomWidget(QWidget *parent = nullptr);

    inline QColor backgroundColor() const { return _backgroundColor; }
    inline void setBackgroundColor(const QColor& value) { _backgroundColor = value; }

private:
    QColor _backgroundColor;

    void paintEvent(QPaintEvent *event) override;
};

}

