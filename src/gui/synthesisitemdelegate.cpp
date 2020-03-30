#include "synthesisitemdelegate.h"
#include "synchronizeditem.h"

#include <iostream>
#include <cstdlib>

#include <QApplication>
#include <QFileInfo>
#include <QColor>
#include <QMimeDatabase>
#include <QMimeType>
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
        QIcon fileIcon = getIconFromFileName(fileInfo.fileName());
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

QIcon SynthesisItemDelegate::getIconFromFileName(const QString &fileName) const
{
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(fileName, QMimeDatabase::MatchExtension);
    if (mime.name().startsWith("image/")) {
        return QIcon(":/client/resources/icons/document types/file-image");
    }
    else if (mime.name().startsWith("audio/")) {
        return QIcon(":/client/resources/icons/document types/file-audio");
    }
    else if (mime.name().startsWith("video/")) {
        return QIcon(":/client/resources/icons/document types/file-video");
    }
    else if (mime.inherits("application/pdf")) {
        return QIcon(":/client/resources/icons/document types/file-pdf");
    }
    else if (mime.name().startsWith("application/vnd.ms-powerpoint")
             || mime.name().startsWith("application/vnd.openxmlformats-officedocument.presentationml")
             || mime.inherits("application/vnd.oasis.opendocument.presentation")) {
        return QIcon(":/client/resources/icons/document types/file-presentation");
    }
    else if (mime.name().startsWith("application/vnd.ms-excel")
             || mime.name().startsWith("application/vnd.openxmlformats-officedocument.spreadsheetml")
             || mime.inherits("application/vnd.oasis.opendocument.spreadsheet")) {
        return QIcon(":/client/resources/icons/document types/file-sheets");
    }
    else if (mime.inherits("application/zip")
             || mime.inherits("application/gzip")
             || mime.inherits("application/tar")
             || mime.inherits("application/rar")
             || mime.inherits("application/x-bzip2")) {
        return QIcon(":/client/resources/icons/document types/file-zip");
    }
    else if (mime.inherits("text/x-csrc")
             || mime.inherits("text/x-c++src")
             || mime.inherits("text/x-java")
             || mime.inherits("text/x-objcsrc")
             || mime.inherits("text/x-python")
             || mime.inherits("text/asp")
             || mime.inherits("text/html")
             || mime.inherits("text/javascript")
             || mime.inherits("application/x-php")
             || mime.inherits("application/x-perl")) {
        return QIcon(":/client/resources/icons/document types/file-code");
    }
    else if (mime.inherits("text/plain")
             || mime.inherits("text/xml")) {
       return QIcon(":/client/resources/icons/document types/file-text");
    }
    else if (mime.inherits("application/x-msdos-program")) {
        return QIcon(":/client/resources/icons/document types/file-application");
    }

    return QIcon(":/client/resources/icons/document types/file-default");
}

}
