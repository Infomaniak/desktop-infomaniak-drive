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
#include <QUrl>
#include <QWidget>

#include "common/pinstate.h"
#include "syncfileitem.h"
#include "syncresult.h"

namespace OCC {
namespace Utility {
    static const QString linkStyle = QString("color:#0098FF; font-weight: 450; text-decoration:none;");
    static const QString learnMoreLink = QString("learnMoreLink");

    enum systrayPosition {
        Top = 0,
        Bottom,
        Left,
        Right
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

    QIcon getIconWithColor(const QString &path, const QColor &color = QColor());
    QIcon getIconMenuWithColor(const QString &path, const QColor &color = QColor());

    systrayPosition getSystrayPosition(QScreen *screen);
    bool isPointInSystray(QScreen *screen, const QPoint &point);

    void setStyle(QApplication *app);
    void setStyle(QApplication *app, bool darkTheme);

    QString getFileStatusIconPath(SyncFileItem::Status status);
    QString getFolderStatusIconPath(bool paused, OCC::SyncResult::Status status);
    QString getFolderStatusText(bool paused, bool unresolvedConflicts, SyncResult::Status status,
                                    qint64 currentFile, qint64 totalFiles, qint64 estimatedRemainingTime);
    QString getAccountStatusIconPath(bool paused, OCC::SyncResult::Status status);
    QString getAccountStatusText(bool paused, bool unresolvedConflicts, SyncResult::Status status);
    bool getPauseActionAvailable(bool paused, OCC::SyncResult::Status status, qint64 totalFiles);
    bool getResumeActionAvailable(bool paused, OCC::SyncResult::Status status, qint64 totalFiles);
    bool getSyncActionAvailable(bool paused, OCC::SyncResult::Status status, qint64 totalFiles);
    void pauseSync(const QString &accountid, bool pause);
    void runSync(const QString &accountid);

} // namespace Utility
} // namespace OCC

#endif
