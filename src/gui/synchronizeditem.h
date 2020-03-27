#pragma once

#include <QDate>
#include <QIcon>
#include <QString>

namespace KDC {

class SynchronizedItem
{
public:
    SynchronizedItem() = default;
    ~SynchronizedItem() = default;
    SynchronizedItem(const SynchronizedItem &item) = default;
    SynchronizedItem(const QString &name, const QDate &date);

    inline QString name() const { return _name; };
    inline QDate date() const { return _date; };

private:
    QString _name;
    QDate _date;
};

}

Q_DECLARE_METATYPE(KDC::SynchronizedItem)
