#pragma once

#include <QPainter>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>

namespace KDC {

class SynthesisItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    SynthesisItemDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

}
