#include "synchronizeditem.h"

namespace KDC {

SynchronizedItem::SynchronizedItem(const QString &folderId, const QString &filePath, const QByteArray &fileId,
                                   const QDateTime &dateTime, OCC::SyncFileItem::Status status,
                                   OCC::SyncFileItem::Direction direction)
    : _folderId(folderId)
    , _filePath(filePath)
    , _fileId(fileId)
    , _dateTime(dateTime)
    , _status(status)
    , _direction(direction)
{
}

}
