/*
Infomaniak Drive
Copyright (C) 2020 christophe.larchier@infomaniak.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "synchronizeditemwidget.h"
#include "menuitemwidget.h"
#include "menuwidget.h"
#include "guiutility.h"

#include <QApplication>
#include <QBoxLayout>
#include <QFile>
#include <QFileInfo>
#include <QGraphicsDropShadowEffect>
#include <QMimeDatabase>
#include <QMimeType>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QTimer>
#include <QGraphicsScene>
#include <QGraphicsSvgItem>
#include <QWidgetAction>

namespace KDC {

static const int cornerRadius = 10;
static const int hMargin= 15;
static const int vMargin = 5;
static const int boxHMargin= 12;
static const int boxVMargin = 5;
static const int boxSpacing = 12;
static const int toolBarHSpacing = 10;
static const int buttonsVSpacing = 5;
static const int textSpacing = 10;
static const int statusIconWidth = 10;
static const int shadowBlurRadius = 20;
static const QString dateFormat = "d MMM - HH:mm";
static const int fileNameMaxSize = 32;
static const int hoverStartTimer = 250;

SynchronizedItemWidget::SynchronizedItemWidget(const SynchronizedItem &item, QWidget *parent)
    : QWidget(parent)
    , _item(item)
    , _isWaitingTimer(false)
    , _isSelected(false)
    , _fileIconSize(QSize())
    , _directionIconSize(QSize())
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
    QFileInfo fileInfo(_item.filePath());
    QString fileName = fileInfo.fileName();
    if (fileName.size() > fileNameMaxSize) {
        fileName = fileName.left(fileNameMaxSize) + "...";
    }
    fileNameLabel->setText(fileName);
    vboxText->addStretch();
    vboxText->addWidget(fileNameLabel);

    QHBoxLayout *hboxText = new QHBoxLayout();
    hboxText->setContentsMargins(0, 0, 0, 0);
    hboxText->setSpacing(textSpacing);

    QLabel *fileDateLabel = new QLabel(this);
    fileDateLabel->setObjectName("fileDateLabel");
    fileDateLabel->setText(_item.dateTime().toString(dateFormat));
    hboxText->addWidget(fileDateLabel);

    _fileDirectionLabel = new QLabel(this);
    _fileDirectionLabel->setVisible(false);
    hboxText->addWidget(_fileDirectionLabel);
    hboxText->addStretch();

    vboxText->addLayout(hboxText);
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
    _folderButton->setToolTip(tr("Show in folder"));
    _folderButton->setVisible(false);
    hboxButtons->addWidget(_folderButton);

    _menuButton = new CustomToolButton(this);
    _menuButton->setIconPath(":/client/resources/icons/actions/menu.svg");
    _menuButton->setToolTip(tr("More actions"));
    _menuButton->setVisible(false);
    hboxButtons->addWidget(_menuButton);

    vboxButtons->addLayout(hboxButtons);
    vboxButtons->addStretch();

    hbox->addLayout(vboxButtons);

    connect(this, &SynchronizedItemWidget::fileIconSizeChanged, this, &SynchronizedItemWidget::onFileIconSizeChanged);
    connect(this, &SynchronizedItemWidget::directionIconSizeChanged, this, &SynchronizedItemWidget::onDirectionIconSizeChanged);
    connect(this, &SynchronizedItemWidget::directionIconColorChanged, this, &SynchronizedItemWidget::onDirectionIconColorChanged);
    connect(_folderButton, &CustomToolButton::clicked, this, &SynchronizedItemWidget::onFolderButtonClicked);
    connect(_menuButton, &CustomToolButton::clicked, this, &SynchronizedItemWidget::onMenuButtonClicked);
}

void SynchronizedItemWidget::setSelected(bool isSelected)
{
    _isSelected = isSelected;
    if (_isSelected) {
        bool fileExists = QFile(_item.fullFilePath()).exists();
        _folderButton->setVisible(fileExists);
        _menuButton->setVisible(fileExists);
        _fileDirectionLabel->setVisible(true);

        // Shadow
        QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(this);
        effect->setBlurRadius(shadowBlurRadius);
        effect->setOffset(0);
        effect->setColor(OCC::Utility::getShadowColor());
        setGraphicsEffect(effect);
    }
    else {
        _folderButton->setVisible(false);
        _menuButton->setVisible(false);
        _fileDirectionLabel->setVisible(false);

        setGraphicsEffect(nullptr);
    }

    repaint();
}

void SynchronizedItemWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    // Update shadow color
    QGraphicsDropShadowEffect *effect = qobject_cast<QGraphicsDropShadowEffect *>(graphicsEffect());
    if (effect) {
        effect->setColor(OCC::Utility::getShadowColor());
    }

    if (_isSelected) {
        // Draw round rectangle
        QPainterPath painterPath;
        painterPath.addRoundedRect(rect().marginsRemoved(QMargins(hMargin, vMargin, hMargin, vMargin)),
                                   cornerRadius, cornerRadius);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(Qt::NoPen);
        painter.setBrush(backgroundColorSelection());
        painter.drawPath(painterPath);
    }
}

void SynchronizedItemWidget::enterEvent(QEvent *event)
{
    Q_UNUSED(event)

    _isWaitingTimer = true;
    QTimer::singleShot(hoverStartTimer, [=](){
        if (_isWaitingTimer) {
            setSelected(true);
        }
    });
}

void SynchronizedItemWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)

    _isWaitingTimer = false;
    if (_isSelected) {
        setSelected(false);
    }
}

bool SynchronizedItemWidget::event(QEvent *event)
{
    if (event->type() == QEvent::WindowDeactivate
            || event->type() == QEvent::Hide) {
        setSelected(false);
    }
    return QWidget::event(event);
}

QString SynchronizedItemWidget::getFileIconPathFromFileName(const QString &fileName, ItemType type) const
{
    if (type == ItemType::ItemTypeDirectory) {
        return QString(":/client/resources/icons/document types/folder.svg");
    }
    else if (type == ItemType::ItemTypeFile) {
        QMimeDatabase db;
        QMimeType mime = db.mimeTypeForFile(fileName, QMimeDatabase::MatchExtension);
        if (mime.name().startsWith("image/")) {
            return QString(":/client/resources/icons/document types/file-image.svg");
        }
        else if (mime.name().startsWith("audio/")) {
            return QString(":/client/resources/icons/document types/file-audio.svg");
        }
        else if (mime.name().startsWith("video/")) {
            return QString(":/client/resources/icons/document types/file-video.svg");
        }
        else if (mime.inherits("application/pdf")) {
            return QString(":/client/resources/icons/document types/file-pdf.svg");
        }
        else if (mime.name().startsWith("application/vnd.ms-powerpoint")
                 || mime.name().startsWith("application/vnd.openxmlformats-officedocument.presentationml")
                 || mime.inherits("application/vnd.oasis.opendocument.presentation")) {
            return QString(":/client/resources/icons/document types/file-presentation.svg");
        }
        else if (mime.name().startsWith("application/vnd.ms-excel")
                 || mime.name().startsWith("application/vnd.openxmlformats-officedocument.spreadsheetml")
                 || mime.inherits("application/vnd.oasis.opendocument.spreadsheet")) {
            return QString(":/client/resources/icons/document types/file-sheets.svg");
        }
        else if (mime.inherits("application/zip")
                 || mime.inherits("application/gzip")
                 || mime.inherits("application/tar")
                 || mime.inherits("application/rar")
                 || mime.inherits("application/x-bzip2")) {
            return QString(":/client/resources/icons/document types/file-zip.svg");
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
            return QString(":/client/resources/icons/document types/file-code.svg");
        }
        else if (mime.inherits("text/plain")
                 || mime.inherits("text/xml")) {
           return QString(":/client/resources/icons/document types/file-text.svg");
        }
        else if (mime.inherits("application/x-msdos-program")) {
            return QString(":/client/resources/icons/document types/file-application.svg");
        }

        return QString(":/client/resources/icons/document types/file-default.svg");
    }
    else {
        return QString(":/client/resources/icons/document types/file-default.svg");
    }
}

QIcon SynchronizedItemWidget::getIconWithStatus(const QString &filePath, ItemType type, OCC::SyncFileItem::Status status)
{
    QGraphicsSvgItem *fileItem = new QGraphicsSvgItem(getFileIconPathFromFileName(filePath, type));
    QGraphicsSvgItem *statusItem = new QGraphicsSvgItem(OCC::Utility::getFileStatusIconPath(status));

    QGraphicsScene scene;
    scene.setSceneRect(QRectF(QPointF(0, 0), _fileIconSize));

    scene.addItem(fileItem);
    fileItem->setPos(QPointF(statusIconWidth / 2, statusIconWidth / 2));

    scene.addItem(statusItem);
    statusItem->setPos(QPointF(_fileIconSize.width() - statusIconWidth, _fileIconSize.width() - statusIconWidth));
    qreal statusItemScale = statusIconWidth / statusItem->boundingRect().width();
    statusItem->setScale(statusItemScale);

    Q_CHECK_PTR(qApp->primaryScreen());
    qreal ratio = qApp->primaryScreen()->devicePixelRatio();
    QPixmap pixmap(QSize(scene.width() * ratio, scene.height() * ratio));
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    scene.render(&painter);

    QIcon iconWithStatus;
    iconWithStatus.addPixmap(pixmap);

    return iconWithStatus;
}

void SynchronizedItemWidget::setDirectionIcon()
{
    if (_fileDirectionLabel && _directionIconSize != QSize() && _directionIconColor != QColor()) {
        switch (_item.direction()) {
        case OCC::SyncFileItem::Direction::None:
            break;
        case OCC::SyncFileItem::Direction::Up:
            _fileDirectionLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/upload.svg", _directionIconColor)
                                          .pixmap(_directionIconSize));
            break;
        case OCC::SyncFileItem::Direction::Down:
            _fileDirectionLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/download.svg", _directionIconColor)
                                          .pixmap(_directionIconSize));
            break;
        }
    }
}

void SynchronizedItemWidget::onFileIconSizeChanged()
{
    if (_fileIconLabel) {
        QFileInfo fileInfo(_item.fullFilePath());
        QIcon fileIconWithStatus = getIconWithStatus(fileInfo.fileName(), _item.type(), _item.status());
        _fileIconLabel->setPixmap(fileIconWithStatus.pixmap(_fileIconSize));
    }
}

void SynchronizedItemWidget::onDirectionIconSizeChanged()
{
    setDirectionIcon();
}

void SynchronizedItemWidget::onDirectionIconColorChanged()
{
    setDirectionIcon();
}

void SynchronizedItemWidget::onFolderButtonClicked()
{
    emit openFolder(_item);
}

void SynchronizedItemWidget::onMenuButtonClicked()
{
    if (_menuButton) {
        MenuWidget *menu = new MenuWidget(MenuWidget::Menu, this);

        QWidgetAction *openAction = new QWidgetAction(this);
        MenuItemWidget *openMenuItemWidget = new MenuItemWidget(tr("Open"));
        openMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/view.svg");
        openAction->setDefaultWidget(openMenuItemWidget);
        connect(openAction, &QWidgetAction::triggered, this, &SynchronizedItemWidget::onOpenActionTriggered);
        menu->addAction(openAction);

        QWidgetAction *favoritesAction = new QWidgetAction(this);
        MenuItemWidget *favoritesMenuItemWidget = new MenuItemWidget(tr("Add to favorites"));
        favoritesMenuItemWidget->hide();
        favoritesMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/favorite.svg");
        favoritesAction->setDefaultWidget(favoritesMenuItemWidget);
        connect(favoritesAction, &QWidgetAction::triggered, this, &SynchronizedItemWidget::onFavoritesActionTriggered);
        menu->addAction(favoritesAction);

        QWidgetAction *rightsAndSharingAction = new QWidgetAction(this);
        MenuItemWidget *rightsAndSharingMenuItemWidget = new MenuItemWidget(tr("Rights and sharing"));
        rightsAndSharingMenuItemWidget->hide();
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

        menu->exec(QWidget::mapToGlobal(_menuButton->geometry().center()));
    }
}

void SynchronizedItemWidget::onOpenActionTriggered(bool checked)
{
    Q_UNUSED(checked)

    emit open(_item);
}

void SynchronizedItemWidget::onFavoritesActionTriggered(bool checked)
{
    Q_UNUSED(checked)

    emit addToFavourites(_item);
}

void SynchronizedItemWidget::onRightAndSharingActionTriggered(bool checked)
{
    Q_UNUSED(checked)

    emit manageRightAndSharing(_item);
}

void SynchronizedItemWidget::onCopyLinkActionTriggered(bool checked)
{
    Q_UNUSED(checked)

    emit copyLink(_item);
}

void SynchronizedItemWidget::onDisplayOnDriveActionTriggered(bool checked)
{
    Q_UNUSED(checked)

    emit displayOnWebview(_item);
}

}
