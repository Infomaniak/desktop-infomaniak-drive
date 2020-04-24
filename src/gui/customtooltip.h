#pragma once

#include <QDialog>
#include <QPaintEvent>
#include <QPoint>
#include <QString>

namespace KDC {

class CustomToolTip : public QDialog
{
    Q_OBJECT

    Q_PROPERTY(QColor background_color READ backgroundColor WRITE setBackgroundColor)

public:
    CustomToolTip(const QString &text, const QPoint &position, int toolTipDuration, QWidget *parent = nullptr);

    inline QColor backgroundColor() const { return _backgroundColor; }
    inline void setBackgroundColor(const QColor& color) { _backgroundColor = color; }

private:
    QPoint _cursorPosition;
    QColor _backgroundColor;

    void paintEvent(QPaintEvent *event) override;
};

}
