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

#include "customwordwraplabel.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QRect>
#include <QString>

namespace KDC {

CustomWordWrapLabel::CustomWordWrapLabel(QWidget *parent, Qt::WindowFlags f)
    : QLabel(parent, f)
    , _maxWidth(0)
{
    setWordWrap(true);
}

CustomWordWrapLabel::CustomWordWrapLabel(const QString &text, QWidget *parent, Qt::WindowFlags f)
    : QLabel(text, parent, f)
    , _maxWidth(0)
{
    setWordWrap(true);
}

QSize CustomWordWrapLabel::sizeHint() const
{
    if (_maxWidth) {
        QFontMetrics metrics(font());
        QRect textRect = metrics.boundingRect(QApplication::desktop()->geometry(), alignment() | Qt::TextWordWrap, text());
        int nbLines = textRect.width() / _maxWidth + 1;
        return QSize(QLabel::sizeHint().width(),
                     nbLines * textRect.height() + contentsMargins().top() + contentsMargins().bottom());

    }
    return QLabel::sizeHint();
}

}
