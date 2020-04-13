#pragma once

#include <QAction>
#include <QColor>
#include <QMenu>
#include <QPaintEvent>
#include <QSize>
#include <QString>

namespace KDC {

class MenuWidget : public QMenu
{
    Q_OBJECT

    Q_PROPERTY(QColor background_color READ backgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(QColor icon_color READ iconColor WRITE setIconColor)

public:
    static const std::string iconPathProperty;

    MenuWidget(QWidget *parent = nullptr);

    inline QColor backgroundColor() const { return _backgroundColor; }
    inline void setBackgroundColor(const QColor& color) { _backgroundColor = color; }

    inline QColor iconColor() const { return _iconColor; }
    inline void setIconColor(const QColor& color) {
        _iconColor = color;
        emit iconColorChanged();
    }

signals:
    void iconColorChanged();

private:
    QColor _backgroundColor;
    QColor _iconColor;

    void paintEvent(QPaintEvent *event) override;

private slots:
    void onIconColorChanged();
};

}
