#pragma once

#include <QAction>
#include <QColor>
#include <QMenu>
#include <QPaintEvent>
#include <QPoint>
#include <QString>

namespace KDC {

class MenuWidget : public QMenu
{
    Q_OBJECT

    Q_PROPERTY(QColor background_color READ backgroundColor WRITE setBackgroundColor)

public:
    static const std::string actionTypeProperty;

    MenuWidget(QWidget *parent = nullptr);
    MenuWidget(const QString &title, QWidget *parent = nullptr);

    inline QColor backgroundColor() const { return _backgroundColor; }
    inline void setBackgroundColor(const QColor& color) { _backgroundColor = color; }

    QAction *exec(const QPoint &pos, bool offsetAuto = false);

private:
    QColor _backgroundColor;

    void paintEvent(QPaintEvent *event) override;
};

}
