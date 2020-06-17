/*
Infomaniak Drive
Copyright (C) 2020 christophe.larchier@infomaniak.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#pragma once

#include <QDialog>
#include <QPaintEvent>
#include <QPoint>

namespace KDC {

class ErrorsPopup : public QDialog
{
    Q_OBJECT

    Q_PROPERTY(QColor background_color READ backgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(QSize warning_icon_size READ warningIconSize WRITE setWarningIconSize)
    Q_PROPERTY(QColor warning_icon_color READ warningIconColor WRITE setWarningIconColor)
    Q_PROPERTY(QSize arrow_icon_size READ arrowIconSize WRITE setArrowIconSize)
    Q_PROPERTY(QColor arrow_icon_color READ arrowIconColor WRITE setArrowIconColor)

public:
    struct DriveError
    {
        QString accountId;
        QString accountName;
        int errorsCount;
    };

    explicit ErrorsPopup(const QList<DriveError> &driveErrorList, QPoint position, QWidget *parent = nullptr);

signals:
    void accountSelected(const QString &accountId);

private:
    QPoint _position;
    QColor _backgroundColor;
    QSize _warningIconSize;
    QColor _warningIconColor;
    QSize _arrowIconSize;
    QColor _arrowIconColor;
    bool _painted;

    void paintEvent(QPaintEvent *event) override;

    inline QColor backgroundColor() const { return _backgroundColor; }
    inline void setBackgroundColor(const QColor &value) { _backgroundColor = value; }

    inline QSize warningIconSize() const { return _warningIconSize; }
    inline void setWarningIconSize(const QSize &size) {
        _warningIconSize = size;
        setWarningIcon();
    }

    inline QColor warningIconColor() const { return _warningIconColor; }
    inline void setWarningIconColor(const QColor &value) {
        _warningIconColor = value;
        setWarningIcon();
    }

    inline QSize arrowIconSize() const { return _arrowIconSize; }
    inline void setArrowIconSize(const QSize &size) {
        _arrowIconSize = size;
        setArrowIcon();
    }

    inline QColor arrowIconColor() const { return _arrowIconColor; }
    inline void setArrowIconColor(const QColor &value) {
        _arrowIconColor = value;
        setArrowIcon();
    }

    void setWarningIcon();
    void setArrowIcon();

private slots:
    void onActionButtonClicked();
};

}

