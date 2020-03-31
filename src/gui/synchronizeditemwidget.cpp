#include "synchronizeditemwidget.h"
#include "customtoolbutton.h"

#include <QBoxLayout>
#include <QFileInfo>
#include <QIcon>
#include <QLabel>
#include <QMimeDatabase>
#include <QMimeType>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPoint>
#include <QStyleOption>

namespace KDC {

static const int cornerRadius = 10;
static const int hMargin= 15;
static const int vMargin = 0;
static const int boxHMargin= 12;
static const int boxVMargin = 5;
static const int iconWidth = 25;
static const int iconHeight = 25;
static const QString dateFormat = "d MMM - HH:mm";

SynchronizedItemWidget::SynchronizedItemWidget(const SynchronizedItem &item, bool isSelected, QWidget *parent)
    : QWidget(parent)
    , _item(item)
    , _isSelected(isSelected)
    , _fileName(QString())
    , _fileDate(QString())
    , _fileNameLabel(nullptr)
    , _fileDateLabel(nullptr)
    , _fileNameColor(Qt::black)
    , _fileDateColor(Qt::black)
{
    setContentsMargins(hMargin, vMargin, hMargin, vMargin);

    QHBoxLayout *hbox = new QHBoxLayout(this);
    hbox->setContentsMargins(boxHMargin, boxVMargin, boxHMargin, boxVMargin);
    hbox->setSpacing(12);
    setLayout(hbox);

    QLabel *fileIconLabel = new QLabel(this);
    QFileInfo fileInfo(_item.name());
    QIcon fileIcon = getIconFromFileName(fileInfo.fileName());
    fileIconLabel->setPixmap(fileIcon.pixmap(QSize(iconWidth, iconHeight)));
    hbox->addWidget(fileIconLabel);

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);

    QHBoxLayout *hboxNameAndButtons = new QHBoxLayout(this);
    hboxNameAndButtons->setContentsMargins(0, 0, 0, 0);
    hboxNameAndButtons->setSpacing(0);

    _fileNameLabel = new QLabel(this);
    _fileName = fileInfo.fileName();
    hboxNameAndButtons->addWidget(_fileNameLabel);

    hboxNameAndButtons->addStretch();

    CustomToolButton *folderButton = new CustomToolButton(this);
    folderButton->setIconPath(":/client/resources/icons/actions/folder");
    hboxNameAndButtons->addWidget(folderButton);

    CustomToolButton *menuButton = new CustomToolButton(this);
    menuButton->setIconPath(":/client/resources/icons/actions/menu");
    hboxNameAndButtons->addWidget(menuButton);

    vbox->addLayout(hboxNameAndButtons);

    _fileDateLabel = new QLabel(this);
    _fileDate = _item.dateTime().toString(dateFormat);
    vbox->addWidget(_fileDateLabel);
    vbox->addStretch();

    hbox->addLayout(vbox);

    connect(this, &SynchronizedItemWidget::fileNameColorChanged, this, &SynchronizedItemWidget::onFileNameColorChanged);
    connect(this, &SynchronizedItemWidget::fileDateColorChanged, this, &SynchronizedItemWidget::onFileDateColorChanged);
}

void SynchronizedItemWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    // Draw background
    QPainterPath painterPath1;
    painterPath1.addRect(rect());

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(backgroundColor());
    painter.drawPath(painterPath1);

    if (_isSelected) {
        // Draw round rectangle
        QPainterPath painterPath2;
        painterPath2.addRoundedRect(rect().marginsRemoved(QMargins(hMargin, vMargin, hMargin, vMargin)),
                                    cornerRadius, cornerRadius);
        //QPen pen(QColor("#F0F0F0"), 2);
        //painter.setPen(pen);
        painter.setBrush(backgroundColorSelection());
        painter.drawPath(painterPath2);
    }
}

QIcon SynchronizedItemWidget::getIconFromFileName(const QString &fileName) const
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

void SynchronizedItemWidget::onFileNameColorChanged()
{
    if (_fileNameLabel) {
        _fileNameLabel->setText(QString("<font color=\"%1\">%2</font>").arg(_fileNameColor.name()).arg(_fileName));
    }
}

void SynchronizedItemWidget::onFileDateColorChanged()
{
    if (_fileDateLabel) {
        _fileDateLabel->setText(QString("<font color=\"%1\">%2</font>").arg(_fileDateColor.name()).arg(_fileDate));
    }
}

}
