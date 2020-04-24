/*
 * Copyright (C) by Christian Kamm <mail@ckamm.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#include "guiutility.h"

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QFile>
#include <QGraphicsColorizeEffect>
#include <QGraphicsSvgItem>
#include <QGraphicsScene>
#include <QIcon>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QScreen>
#include <QUrlQuery>

#include "common/asserts.h"
#include "common/utility.h"
#include "libcommon/commonutility.h"

using namespace OCC;

static const QString styleSheetWhiteFile(":/client/resources/styles/stylesheetwhite.qss");
static const QString styleSheetBlackFile(":/client/resources/styles/stylesheetblack.qss");

Q_LOGGING_CATEGORY(lcUtility, "gui.utility", QtInfoMsg)

bool Utility::openBrowser(const QUrl &url, QWidget *errorWidgetParent)
{
    if (!QDesktopServices::openUrl(url)) {
        if (errorWidgetParent) {
            QMessageBox::warning(
                errorWidgetParent,
                QCoreApplication::translate("utility", "Could not open browser"),
                QCoreApplication::translate("utility",
                    "There was an error when launching the browser to go to "
                    "URL %1. Maybe no default browser is configured?")
                    .arg(url.toString()));
        }
        qCWarning(lcUtility) << "QDesktopServices::openUrl failed for" << url;
        return false;
    }
    return true;
}

bool Utility::openEmailComposer(const QString &subject, const QString &body, QWidget *errorWidgetParent)
{
    QUrl url(QLatin1String("mailto:"));
    QUrlQuery query;
    query.setQueryItems({ { QLatin1String("subject"), subject },
        { QLatin1String("body"), body } });
    url.setQuery(query);

    if (!QDesktopServices::openUrl(url)) {
        if (errorWidgetParent) {
            QMessageBox::warning(
                errorWidgetParent,
                QCoreApplication::translate("utility", "Could not open email client"),
                QCoreApplication::translate("utility",
                    "There was an error when launching the email client to "
                    "create a new message. Maybe no default email client is "
                    "configured?"));
        }
        qCWarning(lcUtility) << "QDesktopServices::openUrl failed for" << url;
        return false;
    }
    return true;
}

QString Utility::vfsCurrentAvailabilityText(VfsItemAvailability availability)
{
    switch(availability) {
    case VfsItemAvailability::AlwaysLocal:
        return QCoreApplication::translate("utility", "Always available locally");
    case VfsItemAvailability::AllHydrated:
        return QCoreApplication::translate("utility", "Currently available locally");
    case VfsItemAvailability::Mixed:
        return QCoreApplication::translate("utility", "Some available online only");
    case VfsItemAvailability::AllDehydrated:
        return QCoreApplication::translate("utility", "Available online only");
    case VfsItemAvailability::OnlineOnly:
        return QCoreApplication::translate("utility", "Available online only");
    }
    ENFORCE(false);
}

QString Utility::vfsPinActionText()
{
    return QCoreApplication::translate("utility", "Make always available locally");
}

QString Utility::vfsFreeSpaceActionText()
{
    return QCoreApplication::translate("utility", "Free up local space");
}

QIcon Utility::getIconWithColor(const QString &path, const QColor &color)
{
    QGraphicsSvgItem *item = new QGraphicsSvgItem(path);

    if (color.isValid()) {
        QGraphicsColorizeEffect *effect = new QGraphicsColorizeEffect;
        effect->setColor(color);
        effect->setStrength(1);
        item->setGraphicsEffect(effect);
    }

    QGraphicsScene scene;
    scene.addItem(item);

    qreal ratio = qApp->primaryScreen()->devicePixelRatio();
    QPixmap pixmap(QSize(scene.width() * ratio, scene.height() * ratio));
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    scene.render(&painter);

    QIcon icon;
    icon.addPixmap(pixmap);
    return icon;
}

Utility::systrayPosition Utility::getSystrayPosition(QScreen *screen)
{
    QRect displayRect = screen->geometry();
    QRect desktopRect = screen->availableGeometry();
    if (desktopRect.height() < displayRect.height()) {
        if (desktopRect.y() > displayRect.y()) {
            return systrayPosition::Top;
        }
        else {
            return systrayPosition::Bottom;
        }
    }
    else {
        if (desktopRect.x() > displayRect.x()) {
            return systrayPosition::Left;
        }
        else {
            return systrayPosition::Right;
        }
    }
}

bool Utility::isPointInSystray(QScreen *screen, const QPoint &point)
{
    QRect displayRect = screen->geometry();
    QRect desktopRect = screen->availableGeometry();
    if (desktopRect.height() < displayRect.height()) {
        if (desktopRect.y() > displayRect.y()) {
            // Systray position = Top
            return point.y() < desktopRect.y();
        }
        else {
            // Systray position = Bottom
            return point.y() > desktopRect.y() + desktopRect.height();
        }
    }
    else {
        if (desktopRect.x() > displayRect.x()) {
            // Systray position = Left
            return point.x() < desktopRect.x();
        }
        else {
            // Systray position = Right
            return point.x() > desktopRect.x() + desktopRect.width();
        }
    }

}

QIcon Utility::getIconMenuWithColor(const QString &path, const QColor &color)
{
    QGraphicsSvgItem *item = new QGraphicsSvgItem(path);
    QGraphicsSvgItem *itemMenu = new QGraphicsSvgItem(":/client/resources/icons/actions/chevron-down.svg");

    if (color.isValid()) {
        QGraphicsColorizeEffect *effect = new QGraphicsColorizeEffect;
        effect->setColor(color);
        effect->setStrength(1);
        item->setGraphicsEffect(effect);

        QGraphicsColorizeEffect *effectMenu = new QGraphicsColorizeEffect;
        effectMenu->setColor(color);
        effectMenu->setStrength(1);
        itemMenu->setGraphicsEffect(effectMenu);
    }

    QGraphicsScene scene;
    scene.addItem(item);
    item->setPos(QPointF(0, 0));
    int iconWidth = scene.width();
    scene.setSceneRect(QRectF(0, 0, iconWidth * 2, iconWidth));

    scene.addItem(itemMenu);
    itemMenu->setPos(QPointF(5.0 / 4.0 * iconWidth, 1.0 / 4.0 * iconWidth));
    itemMenu->setScale(0.5);

    qreal ratio = qApp->primaryScreen()->devicePixelRatio();
    QPixmap pixmap(QSize(scene.width() * ratio, scene.height() * ratio));
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    scene.render(&painter);

    QIcon icon;
    icon.addPixmap(pixmap);
    return icon;
}

void Utility::setStyle(QApplication *app)
{
    // Load style sheet
    QFile ssFile(hasDarkSystray() ? styleSheetBlackFile : styleSheetWhiteFile);
    if (ssFile.exists()) {
        ssFile.open(QFile::ReadOnly);
        QString StyleSheet = QLatin1String(ssFile.readAll());
        app->setStyleSheet(StyleSheet);
    }
    else {
        qCWarning(lcUtility) << "Style sheet file not found!";
    }
}

