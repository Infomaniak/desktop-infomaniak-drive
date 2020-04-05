#pragma once

#include <QColor>
#include <QHBoxLayout>
#include <QPaintEvent>
#include <QWidget>

namespace KDC {

class HalfRoundRectWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor bottom_corners_color READ bottomCornersColor WRITE setBottomCornersColor)

public:
    explicit HalfRoundRectWidget(QWidget *parent = nullptr);

    inline QColor bottomCornersColor() const { return _bottomCornersColor; }
    inline void setBottomCornersColor(const QColor &value) { _bottomCornersColor = value; }

    void setContentsMargins(int left, int top, int right, int bottom);
    void addWidget(QWidget *widget, int stretch = 0, Qt::Alignment alignment = Qt::Alignment());
    void addStretch(int stretch = 0);

private:
    QColor _bottomCornersColor;
    QHBoxLayout *_hboxLayout;

    void paintEvent(QPaintEvent *event) override;
};

}
