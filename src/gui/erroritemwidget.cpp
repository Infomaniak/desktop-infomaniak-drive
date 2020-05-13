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

#include "erroritemwidget.h"
#include "guiutility.h"

#include <QBoxLayout>
#include <QFileInfo>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPainterPath>

namespace KDC {

static const int cornerRadius = 10;
static const int hMargin= 20;
static const int vMargin = 5;
static const int boxHMargin= 15;
static const int boxVMargin = 15;
static const int boxHSpacing = 10;
static const int boxVSpacing = 10;
static const int shadowBlurRadius = 20;
static const QSize statusIconSize = QSize(15, 15);
static const QString dateFormat = "d MMM yyyy - HH:mm";

ErrorItemWidget::ErrorItemWidget(const SynchronizedItem &item, const AccountInfo &accountInfo, QWidget *parent)
    : QWidget(parent)
    , _item(item)
    , _backgroundColor(QColor())
{
    setContentsMargins(hMargin, vMargin, hMargin, vMargin);

    QHBoxLayout *hBox = new QHBoxLayout();
    hBox->setContentsMargins(boxHMargin, boxVMargin, boxHMargin, boxVMargin);
    hBox->setSpacing(boxHSpacing);
    setLayout(hBox);

    // Left box
    QVBoxLayout *vBoxLeft = new QVBoxLayout();
    vBoxLeft->setContentsMargins(0, 0, 0, 0);
    vBoxLeft->setMargin(0);
    hBox->addLayout(vBoxLeft);

    QLabel *statusIconLabel = new QLabel(this);
    QString statusIconPath = OCC::Utility::getFileStatusIconPath(_item.status());
    statusIconLabel->setPixmap(OCC::Utility::getIconWithColor(statusIconPath).pixmap(statusIconSize));
    vBoxLeft->addWidget(statusIconLabel);
    vBoxLeft->addStretch();

    // Middle box
    QVBoxLayout *vBoxMiddle = new QVBoxLayout();
    vBoxMiddle->setContentsMargins(0, 0, 0, 0);
    vBoxMiddle->setSpacing(boxVSpacing);
    hBox->addLayout(vBoxMiddle);
    hBox->setStretchFactor(vBoxMiddle, 1);

    QLabel *fileNameLabel = new QLabel(this);
    fileNameLabel->setObjectName("fileNameLabel");
    QFileInfo fileInfo(_item.filePath());
    QString fileName = fileInfo.fileName();
    /*if (fileName.size() > fileNameMaxSize) {
        fileName = fileName.left(fileNameMaxSize) + "...";
    }*/
    fileNameLabel->setText(fileName);
    fileNameLabel->setWordWrap(true);
    vBoxMiddle->addWidget(fileNameLabel);

    QLabel *fileErrorLabel = new QLabel(this);
    fileErrorLabel->setObjectName("fileErrorLabel");
    fileErrorLabel->setText(_item.error());
    fileErrorLabel->setWordWrap(true);
    vBoxMiddle->addWidget(fileErrorLabel);

    QHBoxLayout *hBoxFilePath = new QHBoxLayout();
    hBoxFilePath->setContentsMargins(0, 0, 0, 0);
    hBoxFilePath->setSpacing(boxHSpacing);
    vBoxMiddle->addLayout(hBoxFilePath);

    QLabel *driveIconLabel = new QLabel(this);
    driveIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/drive.svg", accountInfo._color)
                              .pixmap(statusIconSize));
    hBoxFilePath->addWidget(driveIconLabel);

    QLabel *filePathLabel = new QLabel(this);
    filePathLabel->setObjectName("filePathLabel");
    QString filePath = accountInfo._name + "/" + fileInfo.path();
    /*if (filePath.size() > fileNameMaxSize) {
        filePath = filePath.left(fileNameMaxSize) + "...";
    }*/
    filePathLabel->setText(QString("<a style=\"%1\" href=\"ref\">%2</a>")
                           .arg(OCC::Utility::linkStyle)
                           .arg(filePath));
    filePathLabel->setWordWrap(true);
    hBoxFilePath->addWidget(filePathLabel);
    hBoxFilePath->setStretchFactor(filePathLabel, 1);

    // Right box
    QVBoxLayout *vBoxRight = new QVBoxLayout();
    vBoxRight->setContentsMargins(0, 0, 0, 0);
    vBoxRight->setSpacing(0);
    hBox->addLayout(vBoxRight);

    QLabel *fileDateLabel = new QLabel(this);
    fileDateLabel->setObjectName("fileDateLabel");
    fileDateLabel->setText(_item.dateTime().toString(dateFormat));
    vBoxRight->addWidget(fileDateLabel);
    vBoxRight->addStretch();

    // Shadow
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
    effect->setBlurRadius(shadowBlurRadius);
    effect->setOffset(0);
    setGraphicsEffect(effect);

    connect(filePathLabel, &QLabel::linkActivated, this, &ErrorItemWidget::onLinkActivated);
}

void ErrorItemWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    // Update shadow color
    QGraphicsDropShadowEffect *effect = qobject_cast<QGraphicsDropShadowEffect *>(graphicsEffect());
    if (effect) {
        effect->setColor(OCC::Utility::getShadowColor());
    }

    // Draw round rectangle
    QPainterPath painterPath;
    painterPath.addRoundedRect(rect().marginsRemoved(QMargins(hMargin, vMargin, hMargin, vMargin)),
                               cornerRadius, cornerRadius);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(backgroundColor());
    painter.drawPath(painterPath);
}

void ErrorItemWidget::onLinkActivated(const QString &link)
{
    Q_UNUSED(link)

    emit openFolder(_item.fullFilePath());
}

}
