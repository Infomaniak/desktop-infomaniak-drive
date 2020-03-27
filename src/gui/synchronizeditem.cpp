#include "synchronizeditem.h"

namespace KDC {

SynchronizedItem::SynchronizedItem(const QString &name, const QDate &date)
    : _name(name)
    , _date(date)
{
}

}
