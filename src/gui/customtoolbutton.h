#pragma once

#include <QToolButton>

namespace KDC {

class CustomToolButton : public QToolButton
{
    Q_OBJECT

    Q_PROPERTY(QColor icon_color READ iconColor WRITE setIconColor)

public:
    explicit CustomToolButton(QWidget *parent = nullptr);

    inline QString iconPath() const { return _iconPath; };
    inline void setIconPath(const QString &path) { _iconPath = path; }

    inline QColor iconColor() const { return _iconColor; }
    inline void setIconColor(const QColor& color) {
        _iconColor = color;
        emit iconColorChanged();
    }

signals:
    void iconColorChanged();

private:
    QString _iconPath;
    QColor _iconColor;

private slots:
    void onIconColorChanged();
};

}
