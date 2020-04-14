#pragma once

#include <QColor>
#include <QIcon>
#include <QLabel>
#include <QPaintEvent>
#include <QSize>
#include <QString>
#include <QWidget>

namespace KDC {

class MenuItemWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor default_icon_color READ defaultIconColor WRITE setDefaultIconColor)
    Q_PROPERTY(QColor check_icon_color READ checkIconColor WRITE setCheckIconColor)
    Q_PROPERTY(QSize default_icon_size READ defaultIconSize WRITE setDefaultIconSize)
    Q_PROPERTY(QSize submenu_icon_size READ submenuIconSize WRITE setSubmenuIconSize)

public:
    MenuItemWidget(const QString &text, QWidget *parent = nullptr);

    void setLeftIcon(const QString &path, const QColor &color = QColor(), const QSize &size = QSize());
    void setLeftIcon(const QIcon &icon, const QSize &size = QSize());
    void setRightIcon(const QString &path, const QColor &color = QColor(), const QSize &size = QSize());
    void setRightIcon(const QIcon &icon, const QSize &size = QSize());

    inline QColor defaultIconColor() const { return _defaultIconColor; }
    inline void setDefaultIconColor(QColor color)
    {
        _defaultIconColor = color;
        emit defaultIconColorChanged();
    }

    inline QColor checkIconColor() const { return _checkIconColor; }
    inline void setCheckIconColor(QColor color)
    {
        _checkIconColor = color;
        emit checkIconColorChanged();
    }

    inline QSize defaultIconSize() const { return _defaultIconSize; }
    inline void setDefaultIconSize(QSize size)
    {
        _defaultIconSize = size;
        emit defaultIconSizeChanged();
    }

    inline QSize submenuIconSize() const { return _submenuIconSize; }
    inline void setSubmenuIconSize(QSize size)
    {
        _submenuIconSize = size;
        emit submenuIconSizeChanged();
    }

    bool getChecked() const { return _checked; };
    void setChecked(bool value) { _checked = value; };

    bool getHasSubmenu() const { return _hasSubmenu; };
    void setHasSubmenu(bool value) { _hasSubmenu = value; };

signals:
    void defaultIconColorChanged();
    void checkIconColorChanged();
    void defaultIconSizeChanged();
    void submenuIconSizeChanged();

private:
    QString _leftIconPath;
    QColor _leftIconColor;
    QSize _leftIconSize;
    QString _rightIconPath;
    QColor _rightIconColor;
    QSize _rightIconSize;
    QColor _defaultIconColor;
    QColor _checkIconColor;
    QSize _defaultIconSize;
    QSize _submenuIconSize;
    QLabel *_leftIconLabel;
    QLabel *_rightIconLabel;
    bool _checked;
    bool _hasSubmenu;

    void paintEvent(QPaintEvent *paintEvent);

    void setIcons();

private slots:
    void onDefaultIconColorChanged();
    void onCheckIconColorChanged();
    void onDefaultIconSizeChanged();
    void onSubmenuIconSizeChanged();
};

}
