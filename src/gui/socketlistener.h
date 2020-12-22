#pragma once

#include <QBitArray>
#include <QIODevice>
#include <QPointer>

namespace OCC {

class BloomFilter
{
    // Initialize with m=1024 bits and k=2 (high and low 16 bits of a qHash).
    // For a client navigating in less than 100 directories, this gives us a probability less than (1-e^(-2*100/1024))^2 = 0.03147872136 false positives.
    const static int NumBits = 1024;

public:
    BloomFilter();

    void storeHash(uint hash);
    bool isHashMaybeStored(uint hash) const;

private:
    QBitArray hashBits;
};

class SocketListener
{
public:
    QPointer<QIODevice> socket;

    explicit SocketListener(QIODevice *socket);

    void sendMessage(const QString &message, bool doWait = false) const;

    void sendMessageIfDirectoryMonitored(const QString &message, uint systemDirectoryHash) const;

    void registerMonitoredDirectory(uint systemDirectoryHash);

private:
    BloomFilter _monitoredDirectoriesBloomFilter;
};

}
