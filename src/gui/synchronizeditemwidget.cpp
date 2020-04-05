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

namespace KDC {

static const int cornerRadius = 10;
static const int hMargin= 15;
static const int vMargin = 0;
static const int boxHMargin= 12;
static const int boxVMargin = 5;
static const int boxSpacing = 12;
static const QString dateFormat = "d MMM. - HH:mm";

SynchronizedItemWidget::SynchronizedItemWidget(const SynchronizedItem &item, QWidget *parent)
    : QWidget(parent)
    , _item(item)
    , _isSelected(false)
    , _fileIconSize(QSize())
    , _backgroundColor(QColor())
    , _backgroundColorSelection(QColor())
    , _fileIconLabel(nullptr)
{
    setContentsMargins(hMargin, vMargin, hMargin, vMargin);

    QHBoxLayout *hbox = new QHBoxLayout(this);
    hbox->setContentsMargins(boxHMargin, boxVMargin, boxHMargin, boxVMargin);
    hbox->setSpacing(boxSpacing);
    setLayout(hbox);

    _fileIconLabel = new QLabel(this);
    hbox->addWidget(_fileIconLabel);

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);

    QHBoxLayout *hboxNameAndButtons = new QHBoxLayout(this);
    hboxNameAndButtons->setContentsMargins(0, 0, 0, 0);
    hboxNameAndButtons->setSpacing(0);

    QLabel *fileNameLabel = new QLabel(this);
    fileNameLabel->setObjectName("fileNameLabel");
    QFileInfo fileInfo(_item.name());
    fileNameLabel->setText(fileInfo.fileName());
    hboxNameAndButtons->addWidget(fileNameLabel);

    hboxNameAndButtons->addStretch();

    CustomToolButton *folderButton = new CustomToolButton(this);
    folderButton->setIconPath(":/client/resources/icons/actions/folder.svg");
    hboxNameAndButtons->addWidget(folderButton);

    CustomToolButton *menuButton = new CustomToolButton(this);
    menuButton->setIconPath(":/client/resources/icons/actions/menu.svg");
    hboxNameAndButtons->addWidget(menuButton);

    vbox->addLayout(hboxNameAndButtons);

    QLabel *fileDateLabel = new QLabel(this);
    fileDateLabel->setObjectName("fileDateLabel");
    fileDateLabel->setText(_item.dateTime().toString(dateFormat));
    vbox->addWidget(fileDateLabel);
    vbox->addStretch();

    hbox->addLayout(vbox);

    connect(this, &SynchronizedItemWidget::fileIconSizeChanged, this, &SynchronizedItemWidget::onFileIconSizeChanged);
    connect(folderButton, &CustomToolButton::clicked, this, &SynchronizedItemWidget::onFolderButtonClicked);
    connect(menuButton, &CustomToolButton::clicked, this, &SynchronizedItemWidget::onMenuButtonClicked);
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

        painter.setBrush(backgroundColorSelection());
        painter.drawPath(painterPath2);
    }
}

QIcon SynchronizedItemWidget::getIconFromFileName(const QString &fileName) const
{
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(fileName, QMimeDatabase::MatchExtension);
    if (mime.name().startsWith("image/")) {
        return QIcon(":/client/resources/icons/document types/file-image.svg");
    }
    else if (mime.name().startsWith("audio/")) {
        return QIcon(":/client/resources/icons/document types/file-audio.svg");
    }
    else if (mime.name().startsWith("video/")) {
        return QIcon(":/client/resources/icons/document types/file-video.svg");
    }
    else if (mime.inherits("application/pdf")) {
        return QIcon(":/client/resources/icons/document types/file-pdf.svg");
    }
    else if (mime.name().startsWith("application/vnd.ms-powerpoint")
             || mime.name().startsWith("application/vnd.openxmlformats-officedocument.presentationml")
             || mime.inherits("application/vnd.oasis.opendocument.presentation")) {
        return QIcon(":/client/resources/icons/document types/file-presentation.svg");
    }
    else if (mime.name().startsWith("application/vnd.ms-excel")
             || mime.name().startsWith("application/vnd.openxmlformats-officedocument.spreadsheetml")
             || mime.inherits("application/vnd.oasis.opendocument.spreadsheet")) {
        return QIcon(":/client/resources/icons/document types/file-sheets.svg");
    }
    else if (mime.inherits("application/zip")
             || mime.inherits("application/gzip")
             || mime.inherits("application/tar")
             || mime.inherits("application/rar")
             || mime.inherits("application/x-bzip2")) {
        return QIcon(":/client/resources/icons/document types/file-zip.svg");
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
        return QIcon(":/client/resources/icons/document types/file-code.svg");
    }
    else if (mime.inherits("text/plain")
             || mime.inherits("text/xml")) {
       return QIcon(":/client/resources/icons/document types/file-text.svg");
    }
    else if (mime.inherits("application/x-msdos-program")) {
        return QIcon(":/client/resources/icons/document types/file-application.svg");
    }

    return QIcon(":/client/resources/icons/document types/file-default.svg");
}

void SynchronizedItemWidget::onFileIconSizeChanged()
{
    if (_fileIconLabel) {
        QFileInfo fileInfo(_item.name());
        QIcon fileIcon = getIconFromFileName(fileInfo.fileName());
        _fileIconLabel->setPixmap(fileIcon.pixmap(_fileIconSize));
    }
}

void SynchronizedItemWidget::onFolderButtonClicked()
{

}

void SynchronizedItemWidget::onMenuButtonClicked()
{

}

}
