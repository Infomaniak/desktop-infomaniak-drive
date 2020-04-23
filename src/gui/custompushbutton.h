#pragma once

#include <QColor>
#include <QEvent>
#include <QPushButton>
#include <QString>

namespace KDC {

class CustomPushButton : public QPushButton
{
    Q_OBJECT

    Q_PROPERTY(QColor icon_color READ iconColor WRITE setIconColor)
    Q_PROPERTY(QColor icon_color_checked READ iconColorChecked WRITE setIconColorChecked)

public:
    explicit CustomPushButton(QWidget *parent = nullptr);
    explicit CustomPushButton(const QString &text, QWidget *parent = nullptr);

    inline QString iconPath() const { return _iconPath; };
    inline void setIconPath(const QString &path) { _iconPath = path; }

    inline QColor iconColor() const { return _iconColor; }
    inline void setIconColor(const QColor& color) {
        _iconColor = color;
        emit iconColorChanged();
    }

    inline QColor iconColorChecked() const { return _iconColorChecked; }
    inline void setIconColorChecked(const QColor& color) {
        _iconColorChecked = color;
        emit iconColorCheckedChanged();
    }

signals:
    void iconColorChanged();
    void iconColorCheckedChanged();

private:
    QString _iconPath;
    QColor _iconColor;
    QColor _iconColorChecked;

    bool event(QEvent *event);

private slots:
    void onIconColorChanged();
    void onIconColorCheckedChanged();
    void onToggle(bool checked);
};

}
