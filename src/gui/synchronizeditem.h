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
                     const QDateTime &dateTime, OCC::SyncFileItem::Status status,
                     OCC::SyncFileItem::Direction direction);

    inline QString folderId() const { return _folderId; };
    inline QString filePath() const { return _filePath; };
    inline QByteArray fileId() const { return _fileId; };
    inline QDateTime dateTime() const { return _dateTime; };
    inline OCC::SyncFileItem::Status status() const { return _status; };
    inline OCC::SyncFileItem::Direction direction() const { return _direction; };

private:
    QString _folderId;
    QString _filePath;
    QByteArray _fileId;
    QDateTime _dateTime;
    OCC::SyncFileItem::Status _status;
    OCC::SyncFileItem::Direction _direction;
};

}

