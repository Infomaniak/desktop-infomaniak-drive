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
    QColor _backgroundColor;
    QSize _warningIconSize;
    QColor _warningIconColor;

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

    void setWarningIcon();

private slots:
    void onActionButtonClicked(bool checked = false);
};

}

