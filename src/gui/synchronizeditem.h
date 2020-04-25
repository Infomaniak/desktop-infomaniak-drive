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
    SynchronizedItem(const QString &folderId, const QString &filePath, const QByteArray &fileId,
                     OCC::SyncFileItem::Status status, OCC::SyncFileItem::Direction direction,
                     const QString &fullFilePath, const QDateTime &dateTime);

    inline QString folderId() const { return _folderId; };
    inline QString filePath() const { return _filePath; };
    inline QByteArray fileId() const { return _fileId; };
    inline OCC::SyncFileItem::Status status() const { return _status; };
    inline OCC::SyncFileItem::Direction direction() const { return _direction; };
    inline QString fullFilePath() const { return _fullFilePath; };
    inline QDateTime dateTime() const { return _dateTime; };

private:
    QString _folderId;
    QString _filePath;
    QByteArray _fileId;
    OCC::SyncFileItem::Status _status;
    OCC::SyncFileItem::Direction _direction;
    QString _fullFilePath;
    QDateTime _dateTime;
};

}

