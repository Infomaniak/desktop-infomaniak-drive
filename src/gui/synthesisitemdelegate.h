#pragma once

#include <QAbstractItemModel>
#include <QIcon>
#include <QModelIndex>
#include <QPainter>
#include <QSize>
#include <QString>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QWidget>

namespace KDC {

class SynthesisItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    SynthesisItemDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    QIcon getIconFromExt(const QString &ext) const;
};

}
