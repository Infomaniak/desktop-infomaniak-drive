#pragma once

#include <QPainter>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>

namespace KDC {

class SynchronizedItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    SynchronizedItemDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

}
