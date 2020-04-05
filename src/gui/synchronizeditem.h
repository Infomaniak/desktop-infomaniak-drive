#pragma once

#include <QDateTime>
#include <QString>

namespace KDC {

class SynchronizedItem
{
public:
    SynchronizedItem(const QString &name, const QDateTime &dateTime);

    inline QString name() const { return _name; };
    inline QDateTime dateTime() const { return _dateTime; };

private:
    QString _name;
    QDateTime _dateTime;
};

}

