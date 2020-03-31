#include "synchronizeditemdelegate.h"
#include "synchronizeditem.h"
#include "synchronizeditemwidget.h"

#include <iostream>
#include <cstdlib>

#include <QApplication>
#include <QFileInfo>
#include <QColor>

namespace KDC {

SynchronizedItemDelegate::SynchronizedItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void SynchronizedItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QVariant data = index.data();
    if (data.canConvert<SynchronizedItem>()) {
        SynchronizedItem item = qvariant_cast<SynchronizedItem>(data);
        bool isSelected = option.state & QStyle::State_Selected;

        SynchronizedItemWidget widget(item, isSelected);
        widget.resize(option.rect.size());

        // Paint widget
        painter->save();

        painter->translate(option.rect.topLeft());
        widget.render(painter);

        painter->restore();
    }
    else {
        // Should not happen
        QStyledItemDelegate::paint(painter, option, index);
    }
}

}
