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
#include <QImage>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QScreen>
#include <QUrlQuery>

#include "common/asserts.h"
#include "common/utility.h"
#include "libcommon/commonutility.h"
#include "accountmanager.h"
#include "folderman.h"
#include "configfile.h"

using namespace OCC;

static const QString styleSheetWhiteFile(":/client/resources/styles/stylesheetwhite.qss");
static const QString styleSheetBlackFile(":/client/resources/styles/stylesheetblack.qss");
static const QColor styleSheetWhiteShadowColor = QColor(200, 200, 200, 180);
static const QColor styleSheetBlackShadowColor = QColor(20, 20, 20, 180);

Q_LOGGING_CATEGORY(lcGuiUtility, "gui.utility", QtInfoMsg)

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
        qCWarning(lcGuiUtility) << "QDesktopServices::openUrl failed for" << url;
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
        qCWarning(lcGuiUtility) << "QDesktopServices::openUrl failed for" << url;
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
    painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
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
    painter.setRenderHints(QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform, true);
    scene.render(&painter);

    QIcon icon;
    icon.addPixmap(pixmap);
    return icon;
}

void Utility::setStyle(QApplication *app)
{
    setStyle(app, isDarkTheme());
}

void Utility::setStyle(QApplication *app, bool isDarkTheme)
{
    // Load style sheet
    darkTheme = isDarkTheme;
    QFile ssFile(darkTheme ? styleSheetBlackFile : styleSheetWhiteFile);
    if (ssFile.exists()) {
        ssFile.open(QFile::ReadOnly);
        QString StyleSheet = QLatin1String(ssFile.readAll());
        app->setStyleSheet(StyleSheet);
    }
    else {
        qCWarning(lcGuiUtility) << "Style sheet file not found!";
    }
}

QString Utility::getFileStatusIconPath(OCC::SyncFileItem::Status status)
{
    QString path;
    switch (status) {
    case OCC::SyncFileItem::NoStatus:
        path = QString();
        break;
    case OCC::SyncFileItem::FatalError:
    case OCC::SyncFileItem::NormalError:
    case OCC::SyncFileItem::SoftError:
    case OCC::SyncFileItem::DetailError:
    case OCC::SyncFileItem::BlacklistedError:
        path = QString(":/client/resources/icons/statuts/error-sync.svg");
        break;
    case OCC::SyncFileItem::Success:
        path = QString(":/client/resources/icons/statuts/success.svg");
        break;
    case OCC::SyncFileItem::Conflict:
    case OCC::SyncFileItem::FileIgnored:
    case OCC::SyncFileItem::Restoration:
        path = QString(":/client/resources/icons/statuts/warning.svg");
        break;
    }

    return path;
}

QString Utility::getFolderStatusIconPath(bool paused, OCC::SyncResult::Status status)
{
    QString path;
    if (paused || status == OCC::SyncResult::Paused || status == OCC::SyncResult::SyncAbortRequested) {
        path = QString(":/client/resources/icons/statuts/pause.svg");
    }
    else {
        switch (status) {
        case OCC::SyncResult::Undefined:
            path = QString(":/client/resources/icons/statuts/warning.svg");
            break;
        case OCC::SyncResult::NotYetStarted:
        case OCC::SyncResult::SyncRunning:
        case OCC::SyncResult::SyncPrepare:
            path = QString(":/client/resources/icons/statuts/sync.svg");
            break;
        case OCC::SyncResult::Success:
        case OCC::SyncResult::Problem:
            path = QString(":/client/resources/icons/statuts/success.svg");
            break;
        case OCC::SyncResult::Error:
        case OCC::SyncResult::SetupError:
            path = QString(":/client/resources/icons/statuts/error-sync.svg");
            break;
        default:
            break;
        }
    }

    return path;
}


QString Utility::getFolderStatusText(bool paused, bool unresolvedConflicts, SyncResult::Status status,
                                         qint64 currentFile, qint64 totalFiles, qint64 estimatedRemainingTime)
{
    QString text;
    if (paused || status == OCC::SyncResult::Paused || status == OCC::SyncResult::SyncAbortRequested) {
        text = QCoreApplication::translate("utility", "Synchronization paused.");
    }
    else {
        switch (status) {
        case OCC::SyncResult::Undefined:
            text = QCoreApplication::translate("utility", "No folder to synchronize.");
            break;
        case OCC::SyncResult::NotYetStarted:
        case OCC::SyncResult::SyncRunning:
            if (totalFiles > 0) {
                text = QCoreApplication::translate("utility", "Synchronization in progress (%1 on %2)\n%3 left...")
                        .arg(currentFile).arg(totalFiles).arg(OCC::Utility::durationToDescriptiveString1(estimatedRemainingTime));
            }
            else if (status == OCC::SyncResult::NotYetStarted) {
                text = QCoreApplication::translate("utility", "Waiting for synchronization...");
            }
            else {
                text = QCoreApplication::translate("utility", "Synchronization in progress.");
            }
            break;
        case OCC::SyncResult::SyncPrepare:
            text = QCoreApplication::translate("utility", "Preparing to synchronize...");
            break;
        case OCC::SyncResult::Success:
        case OCC::SyncResult::Problem:
            if (unresolvedConflicts) {
                text = QCoreApplication::translate("utility", "You are up to date, unresolved conflicts.");
            }
            else {
                text = QCoreApplication::translate("utility", "You are up to date!");
            }
            break;
        case OCC::SyncResult::Error:
            text = QCoreApplication::translate("utility", "Some files couldn't be synchronized."
                                               " <a style=\"%1\" href=\"%2\">Learn more</a>")
                    .arg(linkStyle)
                    .arg(learnMoreLink);
            break;
        case OCC::SyncResult::SetupError:
            text = QCoreApplication::translate("utility", "Setup error.");
            break;
        default:
            break;
        }
    }

    return text;
}

QString Utility::getAccountStatusIconPath(bool paused, SyncResult::Status status)
{
    return getFolderStatusIconPath(paused, status);
}

QString Utility::getAccountStatusText(bool paused, bool unresolvedConflicts, SyncResult::Status status)
{
    return getFolderStatusText(paused, unresolvedConflicts, status, 0, 0, 0);
}

bool Utility::getPauseActionAvailable(bool paused, SyncResult::Status status, qint64 totalFiles)
{
    if (paused || status == OCC::SyncResult::Paused || status == OCC::SyncResult::SyncAbortRequested) {
        // Pause
        return false;
    }
    else if (totalFiles > 0) {
        // Synchronization in progress
        return true;
    }
    else if (status == OCC::SyncResult::NotYetStarted
             || status == OCC::SyncResult::SyncPrepare
             || status == OCC::SyncResult::SyncRunning
             || status == OCC::SyncResult::Success) {
        return true;
    }

    return false;
}

bool Utility::getResumeActionAvailable(bool paused, SyncResult::Status status, qint64 totalFiles)
{
    Q_UNUSED(totalFiles)

    if (paused || status == OCC::SyncResult::Paused || status == OCC::SyncResult::SyncAbortRequested) {
        // Pause
        return true;
    }

    return false;
}

bool Utility::getSyncActionAvailable(bool paused, SyncResult::Status status, qint64 totalFiles)
{
    if (paused || status == OCC::SyncResult::Paused || status == OCC::SyncResult::SyncAbortRequested) {
        // Pause
        return false;
    }
    else if (totalFiles > 0) {
        // Synchronization in progress
        return true;
    }
    else if (status == OCC::SyncResult::NotYetStarted
             || status == OCC::SyncResult::SyncPrepare
             || status == OCC::SyncResult::SyncRunning
             || status == OCC::SyncResult::Success
             || status == OCC::SyncResult::Problem
             || status == OCC::SyncResult::Error
             || status == OCC::SyncResult::SetupError) {
        return true;
    }

    return false;
}

void Utility::pauseSync(const QString &accountid, bool pause)
{
    // (Un)pause all the folders of (all) the drive
    OCC::FolderMan *folderMan = OCC::FolderMan::instance();
    for (auto folder : folderMan->map()) {
        OCC::AccountPtr folderAccount = folder->accountState()->account();
        if (accountid == QString() || folderAccount->id() == accountid) {
            folder->setSyncPaused(pause);
            if (pause) {
                folder->slotTerminateSync();
            }
        }
    }
}

void Utility::runSync(const QString &accountid)
{
    // Terminate and reschedule any running sync
    OCC::FolderMan *folderMan = OCC::FolderMan::instance();
    for (auto folder : folderMan->map()) {
        if (folder->isSyncRunning()) {
            folder->slotTerminateSync();
            folderMan->scheduleFolder(folder);
        }
    }

    for (auto folder : folderMan->map()) {
        OCC::AccountPtr account = folder->accountState()->account();
        if (account) {
            if (accountid == QString() || account->id() == accountid) {
                folder->slotWipeErrorBlacklist();

                // Insert the selected folder at the front of the queue
                folderMan->scheduleFolderNext(folder);
            }
        }
    }
}

QPixmap Utility::getPixmapFromImage(const QImage &image, const QSize &size)
{
    return QPixmap::fromImage(image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

QColor Utility::getShadowColor()
{
    return darkTheme ? styleSheetBlackShadowColor : styleSheetWhiteShadowColor;
}

bool Utility::isDarkTheme()
{
    bool darkTheme = false;
    if (OCC::Utility::isMac()) {
        darkTheme = hasDarkSystray();
    }
    else {
        ConfigFile cfg;
        darkTheme = cfg.darkTheme();
    }

    return darkTheme;
}
