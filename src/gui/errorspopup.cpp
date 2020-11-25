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

#include "errorspopup.h"
#include "bottomwidget.h"
#include "clickablewidget.h"
#include "guiutility.h"

#include <QBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QLoggingCategory>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>

namespace KDC {

static const int windowWidth = 310;
static const int cornerRadius = 5;
static const int hMargin= 10;
static const int vMargin = 10;
static const int hSpacing = 0;
static const int vSpacing = 10;
static const int boxHMargin= 15;
static const int boxVMargin = 15;
static const int shadowBlurRadius = 20;
static const int menuOffsetX = -30;
static const int menuOffsetY = 10;

const std::string actionTypeProperty = "actionType";

Q_LOGGING_CATEGORY(lcErrorsPopup, "gui.errorspopup", QtInfoMsg)

ErrorsPopup::ErrorsPopup(const QList<DriveError> &driveErrorList, QPoint position, QWidget *parent)
    : QDialog(parent)
    , _position(position)
    , _backgroundColor(QColor())
    , _warningIconSize(QSize())
    , _warningIconColor(QColor())
    , _moved(false)
{
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::X11BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);

    setMinimumWidth(windowWidth);
    setMaximumWidth(windowWidth);

    setContentsMargins(hMargin, vMargin, hMargin, vMargin);

    QVBoxLayout *mainVBox = new QVBoxLayout();
    mainVBox->setContentsMargins(boxHMargin, boxVMargin, boxHMargin, boxVMargin);
    mainVBox->setSpacing(vSpacing);
    setLayout(mainVBox);

    // Title
    QLabel *titleLabel = new QLabel(this);
    titleLabel->setText(tr("Some files couldn't be synchronized on the following kDrive(s) :", "Number of kDrives",
                           driveErrorList.size()));
    titleLabel->setWordWrap(true);
    mainVBox->addWidget(titleLabel);

    // Errors
    for (auto const &driveError : driveErrorList) {
        ClickableWidget *driveWidget = new ClickableWidget(this);
        driveWidget->setProperty(actionTypeProperty.c_str(), driveError.accountId);
        mainVBox->addWidget(driveWidget);

        QHBoxLayout *driveErrorHBox = new QHBoxLayout();
        driveErrorHBox->setContentsMargins(0, 0, 0, 0);
        driveErrorHBox->addSpacing(hSpacing);
        driveWidget->setLayout(driveErrorHBox);

        QLabel *warningIconLabel = new QLabel(this);
        warningIconLabel->setObjectName("warningIconLabel");
        driveErrorHBox->addWidget(warningIconLabel);

        QLabel *driveNameLabel = new QLabel(this);
        QString text = driveError.accountName + QString(tr(" (%n warning(s) or error(s))", "Number of warnings or errors", driveError.errorsCount));
        driveNameLabel->setText(text);
        driveNameLabel->setWordWrap(true);
        driveErrorHBox->addWidget(driveNameLabel);
        driveErrorHBox->addStretch();

        QLabel *arrowIconLabel = new QLabel(this);
        arrowIconLabel->setObjectName("arrowIconLabel");
        driveErrorHBox->addWidget(arrowIconLabel);

        connect(driveWidget, &ClickableWidget::clicked, this, &ErrorsPopup::onActionButtonClicked);
    }

    // Shadow
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(this);
    effect->setBlurRadius(shadowBlurRadius);
    effect->setOffset(0);
    setGraphicsEffect(effect);
}

void ErrorsPopup::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    if (!_moved) {
        // Move menu
        _moved = true;
        QScreen *screen = QGuiApplication::screenAt(_position);
        Q_CHECK_PTR(screen);
        QRect displayRect = screen->geometry();
        int delta = displayRect.right() - (_position.x() + menuOffsetX + geometry().width());
        QPoint offset = QPoint(menuOffsetX + (delta > 0 ? 0 : delta), menuOffsetY);
        move(_position + offset);
    }

    // Update shadow color
    QGraphicsDropShadowEffect *effect = qobject_cast<QGraphicsDropShadowEffect *>(graphicsEffect());
    if (effect) {
        effect->setColor(OCC::Utility::getShadowColor(true));
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

void ErrorsPopup::setWarningIcon()
{
    if (_warningIconSize != QSize() && _warningIconColor != QColor()) {
        QList<QLabel *> labelList = findChildren<QLabel *>("warningIconLabel");
        for (QLabel *label : labelList) {
            label->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/warning.svg", _warningIconColor)
                             .pixmap(_warningIconSize));
        }
    }
}

void ErrorsPopup::setArrowIcon()
{
    if (_arrowIconSize != QSize() && _arrowIconColor != QColor()) {
        QList<QLabel *> labelList = findChildren<QLabel *>("arrowIconLabel");
        for (QLabel *label : labelList) {
            label->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/arrow-right.svg", _arrowIconColor)
                             .pixmap(_arrowIconSize));
        }
    }
}

void ErrorsPopup::onActionButtonClicked()
{
    QString accountId = qvariant_cast<QString>(sender()->property(actionTypeProperty.c_str()));
    emit accountSelected(accountId);
    done(QDialog::Accepted);
}

}
