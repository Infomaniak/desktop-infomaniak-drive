#include "synthesisitemdelegate.h"
#include "synchronizeditem.h"

#include <iostream>
#include <cstdlib>

#include <QApplication>
#include <QFileInfo>
#include <QColor>
#include <QMimeDatabase>
#include <QMimeType>

namespace KDC {

SynthesisItemDelegate::SynthesisItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void SynthesisItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QVariant data = index.data();
    if (data.canConvert<SynchronizedItem>()) {
        SynchronizedItem item = qvariant_cast<SynchronizedItem>(data);
        bool isSelected = option.state & QStyle::State_Selected;

        painter->save();






        painter->restore();
    }
    else {
        // Should not happen
        QStyledItemDelegate::paint(painter, option, index);
    }
}

}
