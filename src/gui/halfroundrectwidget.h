#pragma once

#include <QWidget>
#include <QBoxLayout>
#include <QPaintEvent>

namespace KDC {

class HalfRoundRectWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor bottom_corners_color READ bottomCornersColor WRITE setBottomCornersColor DESIGNABLE true SCRIPTABLE true)

public:
    explicit HalfRoundRectWidget(QWidget *parent = nullptr);

    inline QColor bottomCornersColor() const { return _bottomCornersColor; }
    inline void setBottomCornersColor(const QColor& value) { _bottomCornersColor = value; }

    QHBoxLayout *getLayout() { return _hboxLayout; }

private:
    QColor _bottomCornersColor;
    QHBoxLayout *_hboxLayout;

    void paintEvent(QPaintEvent *event) override;
};

}
