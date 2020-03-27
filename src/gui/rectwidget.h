#pragma once

#include <QWidget>
#include <QBoxLayout>
#include <QPaintEvent>

namespace KDC {

class RectWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor background_color READ backgroundColor WRITE setBackgroundColor DESIGNABLE true SCRIPTABLE true)

public:
    explicit RectWidget(QWidget *parent = nullptr);

    inline QColor backgroundColor() const { return _backgroundColor; }
    inline void setBackgroundColor(const QColor& value) { _backgroundColor = value; }

    QHBoxLayout *getLayout() { return _hboxLayout; }

private:
    QColor _backgroundColor;
    QHBoxLayout *_hboxLayout;

    void paintEvent(QPaintEvent *event) override;
};

}

