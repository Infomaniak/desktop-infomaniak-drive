#include "synchronizeditemwidget.h"
#include "menuitemwidget.h"
#include "menuwidget.h"

#include <QBoxLayout>
#include <QFileInfo>
#include <QIcon>
#include <QLabel>
#include <QMimeDatabase>
#include <QMimeType>
#include <QPainter>
#include <QPainterPath>
#include <QWidgetAction>

namespace KDC {

static const int cornerRadius = 10;
static const int hMargin= 15;
static const int vMargin = 0;
static const int boxHMargin= 12;
static const int boxVMargin = 5;
static const int boxSpacing = 12;
static const int toolBarHSpacing = 10;
static const int buttonsVSpacing = 5;
static const QString dateFormat = "d MMM - HH:mm";

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

    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->setContentsMargins(boxHMargin, boxVMargin, boxHMargin, boxVMargin);
    hbox->setSpacing(boxSpacing);
    setLayout(hbox);

    _fileIconLabel = new QLabel(this);
    hbox->addWidget(_fileIconLabel);

    QVBoxLayout *vboxText = new QVBoxLayout();
    vboxText->setContentsMargins(0, 0, 0, 0);
    vboxText->setSpacing(0);

    QLabel *fileNameLabel = new QLabel(this);
    fileNameLabel->setObjectName("fileNameLabel");
    QFileInfo fileInfo(_item.name());
    fileNameLabel->setText(fileInfo.fileName());
    vboxText->addStretch();
    vboxText->addWidget(fileNameLabel);

    QLabel *fileDateLabel = new QLabel(this);
    fileDateLabel->setObjectName("fileDateLabel");
    fileDateLabel->setText(_item.dateTime().toString(dateFormat));
    vboxText->addWidget(fileDateLabel);
    vboxText->addStretch();

    hbox->addLayout(vboxText);
    hbox->addStretch();

    QVBoxLayout *vboxButtons = new QVBoxLayout();
    vboxButtons->setContentsMargins(0, 0, 0, 0);
    vboxButtons->addSpacing(buttonsVSpacing);

    QHBoxLayout *hboxButtons = new QHBoxLayout();
    hboxButtons->setContentsMargins(0, 0, 0, 0);
    hboxButtons->setSpacing(toolBarHSpacing);

    _folderButton = new CustomToolButton(this);
    _folderButton->setIconPath(":/client/resources/icons/actions/folder.svg");
    _folderButton->setVisible(false);
    hboxButtons->addWidget(_folderButton);

    _menuButton = new CustomToolButton(this);
    _menuButton->setIconPath(":/client/resources/icons/actions/menu.svg");
    _menuButton->setVisible(false);
    hboxButtons->addWidget(_menuButton);

    vboxButtons->addLayout(hboxButtons);
    vboxButtons->addStretch();

    hbox->addLayout(vboxButtons);

    connect(this, &SynchronizedItemWidget::fileIconSizeChanged, this, &SynchronizedItemWidget::onFileIconSizeChanged);
    connect(_folderButton, &CustomToolButton::clicked, this, &SynchronizedItemWidget::onFolderButtonClicked);
    connect(_menuButton, &CustomToolButton::clicked, this, &SynchronizedItemWidget::onMenuButtonClicked);
}

void SynchronizedItemWidget::setSelected(bool isSelected)
{
    _isSelected = isSelected;
    _folderButton->setVisible(_isSelected);
    _menuButton->setVisible(_isSelected);
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
    if (_menuButton) {
        MenuWidget *menu = new MenuWidget(this);

        QWidgetAction *openAction = new QWidgetAction(this);
        MenuItemWidget *openMenuItemWidget = new MenuItemWidget(tr("Open"));
        openMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/view.svg");
        openAction->setDefaultWidget(openMenuItemWidget);
        connect(openAction, &QWidgetAction::triggered, this, &SynchronizedItemWidget::onOpenActionTriggered);
        menu->addAction(openAction);

        QWidgetAction *favoritesAction = new QWidgetAction(this);
        MenuItemWidget *favoritesMenuItemWidget = new MenuItemWidget(tr("Add to favorites"));
        favoritesMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/favorite.svg");
        favoritesAction->setDefaultWidget(favoritesMenuItemWidget);
        connect(favoritesAction, &QWidgetAction::triggered, this, &SynchronizedItemWidget::onFavoritesActionTriggered);
        menu->addAction(favoritesAction);

        QWidgetAction *rightsAndSharingAction = new QWidgetAction(this);
        MenuItemWidget *rightsAndSharingMenuItemWidget = new MenuItemWidget(tr("Rights and sharing"));
        rightsAndSharingMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/share.svg");
        rightsAndSharingAction->setDefaultWidget(rightsAndSharingMenuItemWidget);
        connect(rightsAndSharingAction, &QWidgetAction::triggered, this, &SynchronizedItemWidget::onRightAndSharingActionTriggered);
        menu->addAction(rightsAndSharingAction);

        QWidgetAction *copyLinkAction = new QWidgetAction(this);
        MenuItemWidget *copyLinkMenuItemWidget = new MenuItemWidget(tr("Copy sharing link"));
        copyLinkMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/link.svg");
        copyLinkAction->setDefaultWidget(copyLinkMenuItemWidget);
        connect(copyLinkAction, &QWidgetAction::triggered, this, &SynchronizedItemWidget::onCopyLinkActionTriggered);
        menu->addAction(copyLinkAction);

        menu->addSeparator();

        QWidgetAction *displayOnDriveAction = new QWidgetAction(this);
        MenuItemWidget *displayOnDrivekMenuItemWidget = new MenuItemWidget(tr("Display on drive.infomaniak.com"));
        displayOnDrivekMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/webview.svg");
        displayOnDriveAction->setDefaultWidget(displayOnDrivekMenuItemWidget);
        connect(displayOnDriveAction, &QWidgetAction::triggered, this, &SynchronizedItemWidget::onDisplayOnDriveActionTriggered);
        menu->addAction(displayOnDriveAction);

        menu->exec(QWidget::mapToGlobal(_menuButton->geometry().center()), true);
    }

}

void SynchronizedItemWidget::onOpenActionTriggered(bool checked)
{
    Q_UNUSED(checked)
}

void SynchronizedItemWidget::onFavoritesActionTriggered(bool checked)
{
    Q_UNUSED(checked)
}

void SynchronizedItemWidget::onRightAndSharingActionTriggered(bool checked)
{
    Q_UNUSED(checked)
}

void SynchronizedItemWidget::onCopyLinkActionTriggered(bool checked)
{
    Q_UNUSED(checked)
}

void SynchronizedItemWidget::onDisplayOnDriveActionTriggered(bool checked)
{
    Q_UNUSED(checked)
}

}
