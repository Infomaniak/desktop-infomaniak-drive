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

#include "syncfileitem.h"

#include <QByteArray>
#include <QDateTime>
#include <QString>

namespace KDC {

class SynchronizedItem
{
public:
#if (QT_VERSION < QT_VERSION_CHECK(5, 13, 0))
    // For use with QVector
    SynchronizedItem() = default;

    SynchronizedItem(const SynchronizedItem &item)
    {
        *this = item;
    }

    SynchronizedItem& operator= (const SynchronizedItem& item)
    {
        _folderId = item._folderId;
        _filePath = item._filePath;
        _fileId = item._fileId;
        _status = item._status;
        _direction = item._direction;
        _type = item._type;
        _fullFilePath = item._fullFilePath;
        _dateTime = item._dateTime;
        _error = item._error;
        return *this;
    }
#endif

    SynchronizedItem(const QString &folderId, const QString &filePath, const QByteArray &fileId,
                     OCC::SyncFileItem::Status status, OCC::SyncFileItem::Direction direction,
                     ItemType type, const QString &fullFilePath, const QDateTime &dateTime,
                     const QString &error = QString());

    inline QString folderId() const { return _folderId; };
    inline QString filePath() const { return _filePath; };
    inline QByteArray fileId() const { return _fileId; };
    inline OCC::SyncFileItem::Status status() const { return _status; };
    inline OCC::SyncFileItem::Direction direction() const { return _direction; };
    inline ItemType type() const { return _type; };
    inline QString fullFilePath() const { return _fullFilePath; };
    inline QDateTime dateTime() const { return _dateTime; };
    inline QString error() const { return _error; };

private:
    QString _folderId;
    QString _filePath;
    QByteArray _fileId;
    OCC::SyncFileItem::Status _status;
    OCC::SyncFileItem::Direction _direction;
    ItemType _type;
    QString _fullFilePath;
    QDateTime _dateTime;
    QString _error;
};

}

