#include "synchronizeditem.h"

namespace KDC {

SynchronizedItem::SynchronizedItem(const QString &folderId, const QString &filePath, const QByteArray &fileId,
                                   OCC::SyncFileItem::Status status, OCC::SyncFileItem::Direction direction,
                                   const QString &fullFilePath, const QDateTime &dateTime)
    : _folderId(folderId)
    , _filePath(filePath)
    , _fileId(fileId)
    , _status(status)
    , _direction(direction)
    , _fullFilePath(fullFilePath)
    , _dateTime(dateTime)
{
}

}
