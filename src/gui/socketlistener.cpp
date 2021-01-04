#include "socketlistener.h"

#include <QLoggingCategory>

namespace OCC {

Q_LOGGING_CATEGORY(lcSocketListener, "gui.socketlistener", QtInfoMsg)

BloomFilter::BloomFilter()
    : hashBits(NumBits)
{
}

void BloomFilter::storeHash(uint hash)
{
    hashBits.setBit((hash & 0xFFFF) % NumBits);
    hashBits.setBit((hash >> 16) % NumBits);
}

bool BloomFilter::isHashMaybeStored(uint hash) const
{
    return hashBits.testBit((hash & 0xFFFF) % NumBits)
            && hashBits.testBit((hash >> 16) % NumBits);
}

SocketListener::SocketListener(QIODevice *socket)
    : socket(socket)
{
}

void SocketListener::sendMessage(const QString &message, bool doWait) const
{
    if (!socket) {
        qCInfo(lcSocketListener) << "Not sending message to dead socket:" << message;
        return;
    }

    qCInfo(lcSocketListener) << "Sending SocketAPI message -->" << message << "to" << socket;
    QString localMessage = message;
    if (!localMessage.endsWith(QLatin1Char('\n'))) {
        localMessage.append(QLatin1Char('\n'));
    }

    QByteArray bytesToSend = localMessage.toUtf8();
    qint64 sent = socket->write(bytesToSend);
    if (doWait) {
        socket->waitForBytesWritten(1000);
    }
    if (sent != bytesToSend.length()) {
        qCWarning(lcSocketListener) << "Could not send all data on socket for " << localMessage;
    }
}

void SocketListener::sendMessageIfDirectoryMonitored(const QString &message, uint systemDirectoryHash) const
{
    if (_monitoredDirectoriesBloomFilter.isHashMaybeStored(systemDirectoryHash))
        sendMessage(message, false);
}

void SocketListener::registerMonitoredDirectory(uint systemDirectoryHash)
{
    _monitoredDirectoriesBloomFilter.storeHash(systemDirectoryHash);
}

}
