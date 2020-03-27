#include "synthesisitemdelegate.h"
#include "synchronizeditem.h"

#include <QApplication>
#include <QFileInfo>
#include <QColor>
#include <QPainterPath>

namespace KDC {

static const int cornerRadius = 10;
static const int widgetHeight = 60;
static const int widgetMarginX = 15;
static const int iconPositionX = 12;
static const int iconWidth = 25;
static const int iconHeight = 25;

SynthesisItemDelegate::SynthesisItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void SynthesisItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QVariant data = index.data();
    if (data.canConvert<SynchronizedItem>()) {
        SynchronizedItem item = qvariant_cast<SynchronizedItem>(data);

        painter->save();

        // Draw round rectangle
        QPainterPath painterPath;
        painterPath.addRoundedRect(option.rect.marginsRemoved(QMargins(widgetMarginX, 0, widgetMarginX, 0)),
                                   cornerRadius,
                                   cornerRadius);
        painterPath.addText(QPointF(20, option.rect.height() / 2.0), option.font, item.name());

        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setBrush(QColor("#35393D"));
        painter->setPen(Qt::NoPen); // No border
        painter->drawPath(painterPath);

        // Draw icon
        QFileInfo fileInfo(item.name());
        QString fileExt = fileInfo.completeSuffix();
        QIcon fileIcon = getIconFromExt(fileExt);
        QRectF fileIconRect = QRectF(
                    option.rect.left() + widgetMarginX + iconPositionX,
                    option.rect.top() + (option.rect.height() - iconHeight) / 2.0,
                    iconWidth,
                    iconHeight);
        QPixmap fileIconPixmap = fileIcon.pixmap(iconWidth, iconHeight, QIcon::Normal);
        painter->drawPixmap(QPoint(fileIconRect.left(), fileIconRect.top()), fileIconPixmap);





        painter->restore();
    }
    else {
        // Should not happen
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize SynthesisItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)

    return QSize(option.rect.width(), widgetHeight);
}

QIcon SynthesisItemDelegate::getIconFromExt(const QString &ext) const
{
    return QIcon(":/client/resources/folder-sync");
}

}
