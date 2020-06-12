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

#include "synchronizeditem.h"

namespace KDC {

SynchronizedItem::SynchronizedItem(const QString &folderId, const QString &filePath, const QByteArray &fileId,
                                   OCC::SyncFileItem::Status status, OCC::SyncFileItem::Direction direction,
                                   ItemType type, const QString &fullFilePath, const QDateTime &dateTime,
                                   const QString &error)
    : _folderId(folderId)
    , _filePath(filePath)
    , _fileId(fileId)
    , _status(status)
    , _direction(direction)
    , _fullFilePath(fullFilePath)
    , _dateTime(dateTime)
    , _type(type)
    , _error(error)
{
}

}
