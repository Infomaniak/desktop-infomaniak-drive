#pragma once

#include <QDateTime>
#include <QIcon>
#include <QString>

namespace KDC {

class SynchronizedItem
{
public:
    SynchronizedItem() = default;
    ~SynchronizedItem() = default;
    SynchronizedItem(const SynchronizedItem &item) = default;
    SynchronizedItem(const QString &name, const QDateTime &dateTime);

    inline QString name() const { return _name; };
    inline QDateTime dateTime() const { return _dateTime; };

private:
    QString _name;
    QDateTime _dateTime;
};

}

Q_DECLARE_METATYPE(KDC::SynchronizedItem)
