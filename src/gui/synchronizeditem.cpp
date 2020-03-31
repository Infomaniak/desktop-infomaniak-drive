#include "synchronizeditem.h"

namespace KDC {

SynchronizedItem::SynchronizedItem(const QString &name, const QDateTime &dateTime)
    : _name(name)
    , _dateTime(dateTime)
{
}

}
