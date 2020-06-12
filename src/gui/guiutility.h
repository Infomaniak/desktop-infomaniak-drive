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

#ifndef GUIUTILITY_H
#define GUIUTILITY_H

#include <QApplication>
#include <QColor>
#include <QIcon>
#include <QPoint>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QWidget>

#include "common/pinstate.h"
#include "syncfileitem.h"
#include "syncresult.h"

namespace OCC {
namespace Utility {
    static const QString linkStyle = QString("color:#0098FF; font-weight:450; text-decoration:none;");
    static const QString learnMoreLink = QString("learnMoreLink");

    enum systrayPosition {
        Top = 0,
        Bottom,
        Left,
        Right
    };

    enum WizardAction {
        OpenFolder = 0,
        OpenParameters,
        AddDrive
    };

    /** Open an url in the browser.
     *
     * If launching the browser fails, display a message.
     */
    bool openBrowser(const QUrl &url, QWidget *errorWidgetParent);

    /** Start composing a new email message.
     *
     * If launching the email program fails, display a message.
     */
    bool openEmailComposer(const QString &subject, const QString &body,
        QWidget *errorWidgetParent);

    /** Returns a translated string indicating the current availability.
     *
     * This will be used in context menus to describe the current state.
     */
    QString vfsCurrentAvailabilityText(VfsItemAvailability availability);

    /** Translated text for "making items always available locally" */
    QString vfsPinActionText();

    /** Translated text for "free up local space" (and unpinning the item) */
    QString vfsFreeSpaceActionText();

    QPixmap getPixmapFromImage(const QImage &image, const QSize &size = QSize());
    QIcon getIconWithColor(const QString &path, const QColor &color = QColor());
    QIcon getIconMenuWithColor(const QString &path, const QColor &color = QColor());

    systrayPosition getSystrayPosition(QScreen *screen);
    bool isPointInSystray(QScreen *screen, const QPoint &point);

    bool isDarkTheme();
    void setStyle(QApplication *app);
    void setStyle(QApplication *app, bool isDarkTheme);

    QString getFileStatusIconPath(SyncFileItem::Status status);
    QString getFolderStatusIconPath(bool paused, OCC::SyncResult::Status status);
    QStringList getFolderStatusText(bool paused, bool unresolvedConflicts, SyncResult::Status status,
                                    qint64 currentFile, qint64 totalFiles, qint64 estimatedRemainingTime);
    QString getAccountStatusIconPath(bool paused, OCC::SyncResult::Status status);
    QStringList getAccountStatusText(bool paused, bool unresolvedConflicts, SyncResult::Status status);
    bool getPauseActionAvailable(bool paused, OCC::SyncResult::Status status);
    bool getResumeActionAvailable(bool paused, OCC::SyncResult::Status status);
    bool getSyncActionAvailable(bool paused, OCC::SyncResult::Status status);
    void pauseSync(const QString &accountId, const QString &folderId, bool pause);
    void runSync(const QString &accountId, const QString &folderId);
    QColor getShadowColor(bool dialog = false);
    QUrl getUrlFromLocalPath(const QString &path);
    int getQFontWeightFromQSSFontWeight(int weight);
    qint64 folderSize(const QString &dirPath);
    bool openFolder(const QString &path);

} // namespace Utility
} // namespace OCC

#endif
