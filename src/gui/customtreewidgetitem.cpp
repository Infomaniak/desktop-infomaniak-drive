/*
Infomaniak Drive
Copyright (C) 2020 christophe.larchier@infomaniak.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "customtreewidgetitem.h"

namespace KDC {

CustomTreeWidgetItem::CustomTreeWidgetItem(int type)
    : QTreeWidgetItem(type)
{
}

CustomTreeWidgetItem::CustomTreeWidgetItem(const QStringList &strings, int type)
    : QTreeWidgetItem(strings, type)
{
}

CustomTreeWidgetItem::CustomTreeWidgetItem(QTreeWidget *view, int type)
    : QTreeWidgetItem(view, type)
{
}

CustomTreeWidgetItem::CustomTreeWidgetItem(QTreeWidgetItem *parent, int type)
    : QTreeWidgetItem(parent, type)
{
}

bool CustomTreeWidgetItem::operator<(const QTreeWidgetItem &other) const
{
    int column = treeWidget()->sortColumn();
    if (column == 1) {
        return data(1, Qt::UserRole).toLongLong() < other.data(1, Qt::UserRole).toLongLong();
    }
    return QTreeWidgetItem::operator<(other);
}

}
