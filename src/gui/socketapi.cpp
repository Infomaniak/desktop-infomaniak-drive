/*
 * Copyright (C) by Dominik Schmidt <dev@dominik-schmidt.de>
 * Copyright (C) by Klaas Freitag <freitag@owncloud.com>
 * Copyright (C) by Roeland Jago Douma <roeland@famdouma.nl>
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

#include "socketapi.h"

#include "config.h"
#include "configfile.h"
#include "folderman.h"
#include "folder.h"
#include "theme.h"
#include "common/syncjournalfilerecord.h"
#include "syncengine.h"
#include "syncfileitem.h"
#include "filesystem.h"
#include "version.h"
#include "account.h"
#include "accountstate.h"
#include "account.h"
#include "capabilities.h"
#include "common/asserts.h"
#include "guiutility.h"
#include "getorcreatepubliclinkshare.h"
#include "thumbnailjob.h"
#include "propagatedownload.h"

#include <array>
#include <QBitArray>
#include <QUrl>
#include <QMetaMethod>
#include <QMetaObject>
#include <QStringList>
#include <QScopedPointer>
#include <QFile>
#include <QDir>
#include <QApplication>
#include <QLocalSocket>
#include <QStringBuilder>
#include <QMessageBox>
#include <QFileDialog>
#include <QClipboard>
#include <QBuffer>

#include <QStandardPaths>

#ifdef Q_OS_MAC
#include <CoreFoundation/CoreFoundation.h>
#endif

// This is the version that is returned when the client asks for the VERSION.
// The first number should be changed if there is an incompatible change that breaks old clients.
// The second number should be changed when there are new features.
#define MIRALL_SOCKET_API_VERSION "1.1"

static inline QString removeTrailingSlash(QString path)
{
    Q_ASSERT(path.endsWith(QLatin1Char('/')));
    path.truncate(path.length() - 1);
    return path;
}

static QString buildMessage(const QString &verb, const QString &path, const QString &status = QString())
{
    QString msg(verb);

    if (!status.isEmpty()) {
        msg.append(QLatin1Char(':'));
        msg.append(status);
    }
    if (!path.isEmpty()) {
        msg.append(QLatin1Char(':'));
        QFileInfo fi(path);
        msg.append(QDir::toNativeSeparators(fi.absoluteFilePath()));
    }
    return msg;
}

namespace OCC {

Q_LOGGING_CATEGORY(lcSocketApi, "gui.socketapi", QtInfoMsg)

struct ListenerHasSocketPred
{
    QIODevice *socket;
    ListenerHasSocketPred(QIODevice *socket)
        : socket(socket)
    {
    }
    bool operator()(const SocketListener &listener) const { return listener.socket == socket; }
};

SocketApi::SocketApi(QObject *parent)
    : QObject(parent)
{
    QString socketPath;

    if (Utility::isWindows()) {
        socketPath = QLatin1String("\\\\.\\pipe\\")
            + QLatin1String(APPLICATION_SHORTNAME)
            + QLatin1String("-")
            + QString::fromLocal8Bit(qgetenv("USERNAME"));
        // TODO: once the windows extension supports multiple
        // client connections, switch back to the theme name
        // See issue #2388
        // + Theme::instance()->appName();
    } else if (Utility::isMac()) {
        // This must match the code signing Team setting of the extension
        // Example for developer builds (with ad-hoc signing identity): "" "com.owncloud.desktopclient" ".socketApi"
        // Example for official signed packages: "9B5WD74GWJ." "com.owncloud.desktopclient" ".socketApi"
        socketPath = SOCKETAPI_TEAM_IDENTIFIER_PREFIX APPLICATION_REV_DOMAIN ".socketApi";
#ifdef Q_OS_MAC
        int ret = 0;
        CFURLRef url = (CFURLRef)CFAutorelease((CFURLRef)CFBundleCopyBundleURL(CFBundleGetMainBundle()));
        QString bundlePath = QUrl::fromCFURL(url).path();
        QString cmd;

        // Tell Finder to use the Extension (checking it from System Preferences -> Extensions)
        cmd = QString("pluginkit -v -e use -i " APPLICATION_REV_DOMAIN ".FinderSyncExt");
        ret = system(cmd.toLocal8Bit());

        // Add it again. This was needed for Mojave to trigger a load.
        cmd = QString("pluginkit -v -a ") + bundlePath + "Contents/PlugIns/FinderSyncExt.appex/";
        ret = system(cmd.toLocal8Bit());

#endif
    } else if (Utility::isLinux() || Utility::isBSD()) {
        QString runtimeDir;
        runtimeDir = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
        socketPath = runtimeDir + "/" + Theme::instance()->appName() + "/socket";
    } else {
        qCWarning(lcSocketApi) << "An unexpected system detected, this probably won't work.";
    }

    SocketApiServer::removeServer(socketPath);
    QFileInfo info(socketPath);
    if (!info.dir().exists()) {
        bool result = info.dir().mkpath(".");
        qCDebug(lcSocketApi) << "creating" << info.dir().path() << result;
        if (result) {
            QFile::setPermissions(socketPath,
                QFile::Permissions(QFile::ReadOwner + QFile::WriteOwner + QFile::ExeOwner));
        }
    }
    if (!_localServer.listen(socketPath)) {
        qCWarning(lcSocketApi) << "can't start server" << socketPath;
    } else {
        qCInfo(lcSocketApi) << "server started, listening at " << socketPath;
    }

    connect(&_localServer, &SocketApiServer::newConnection, this, &SocketApi::slotNewConnection);

    // folder watcher
    connect(FolderMan::instance(), &FolderMan::folderSyncStateChange, this, &SocketApi::slotUpdateFolderView);
}

SocketApi::~SocketApi()
{
    _localServer.close();
    // All remaining sockets will be destroyed with _localServer, their parent
    ASSERT(_listeners.isEmpty() || _listeners.first().socket->parent() == &_localServer);
    _listeners.clear();
}

void SocketApi::slotNewConnection()
{
    // Note that on macOS this is not actually a line-based QIODevice, it's a SocketApiSocket which is our
    // custom message based macOS IPC.
    QIODevice *socket = _localServer.nextPendingConnection();

    if (!socket) {
        return;
    }
    qCInfo(lcSocketApi) << "New connection" << socket;
    connect(socket, &QIODevice::readyRead, this, &SocketApi::slotReadSocket);
    connect(socket, SIGNAL(disconnected()), this, SLOT(onLostConnection()));
    connect(socket, &QObject::destroyed, this, &SocketApi::slotSocketDestroyed);
    ASSERT(socket->readAll().isEmpty());

    _listeners.append(SocketListener(socket));
    SocketListener &listener = _listeners.last();

    foreach (Folder *f, FolderMan::instance()->map()) {
        if (f->canSync()) {
            QString message = buildRegisterPathMessage(removeTrailingSlash(f->path()));
            listener.sendMessage(message);
        }
    }
}

void SocketApi::onLostConnection()
{
    qCInfo(lcSocketApi) << "Lost connection " << sender();
    sender()->deleteLater();

    auto socket = qobject_cast<QIODevice *>(sender());
    ASSERT(socket);
    _listeners.erase(std::remove_if(_listeners.begin(), _listeners.end(), ListenerHasSocketPred(socket)), _listeners.end());
}

void SocketApi::slotSocketDestroyed(QObject *obj)
{
    QIODevice *socket = static_cast<QIODevice *>(obj);
    _listeners.erase(std::remove_if(_listeners.begin(), _listeners.end(), ListenerHasSocketPred(socket)), _listeners.end());
}

void SocketApi::slotReadSocket()
{
    QIODevice *socket = qobject_cast<QIODevice *>(sender());
    ASSERT(socket);

    // Find the SocketListener
    //
    // It's possible for the disconnected() signal to be triggered before
    // the readyRead() signals are received - in that case there won't be a
    // valid listener. We execute the handler anyway, but it will work with
    // a SocketListener that doesn't send any messages.
    static auto noListener = SocketListener(nullptr);
    SocketListener *listener = &noListener;
    auto listenerIt = std::find_if(_listeners.begin(), _listeners.end(), ListenerHasSocketPred(socket));
    if (listenerIt != _listeners.end()) {
        listener = &*listenerIt;
    }

    while (socket->canReadLine()) {
        // Make sure to normalize the input from the socket to
        // make sure that the path will match, especially on OS X.
        QString line = QString::fromUtf8(socket->readLine()).normalized(QString::NormalizationForm_C);
        line.chop(1); // remove the '\n'
        qCInfo(lcSocketApi) << "Received SocketAPI message <--" << line << "from" << socket;
        QByteArray command = line.split(":").value(0).toLatin1();
        QByteArray functionWithArguments = "command_" + command + "(QString,SocketListener*)";
        int indexOfMethod = staticMetaObject.indexOfMethod(functionWithArguments);

        QString argument = line.remove(0, command.length() + 1);
        if (indexOfMethod != -1) {
            staticMetaObject.method(indexOfMethod).invoke(this, Q_ARG(QString, argument), Q_ARG(SocketListener *, listener));
        } else {
            qCWarning(lcSocketApi) << "The command is not supported by this version of the client:" << command << "with argument:" << argument;
        }
    }
}

void SocketApi::slotThumbnailFetched(const int &statusCode, const QByteArray &reply,
                                     unsigned int width, uint64_t iNode, const OCC::SocketListener *listener)
{
    if (statusCode != 200) {
        qCWarning(lcSharing) << "Thumbnail status code: " << statusCode;
        return;
    }

    QPixmap pixmap;
    if (!pixmap.loadFromData(reply)) {
        qCWarning(lcSharing) << "Error in pixmap.loadFromData for file with iNode " << iNode;
        return;
    }

    qCDebug(lcSocketApi) << "Thumbnail fetched - size: " << pixmap.width() << "x" << pixmap.height();
    if (pixmap.width() > pixmap.height()) {
        pixmap = pixmap.scaledToWidth(width, Qt::SmoothTransformation);
    }
    else {
        pixmap = pixmap.scaledToHeight(width, Qt::SmoothTransformation);
    }
    qCDebug(lcSocketApi) << "Thumbnail scaled - size: " << pixmap.width() << "x" << pixmap.height();

    QBuffer pixmapBuffer;
    pixmapBuffer.open(QIODevice::WriteOnly);
    pixmap.save(&pixmapBuffer, "BMP");

    QString pm(pixmapBuffer.data().toBase64());
    listener->sendMessage(QString("GET_THUMBNAIL:%1:%2")
                          .arg(QString::number(iNode))
                          .arg(QString(pixmapBuffer.data().toBase64())));
}

void SocketApi::slotRegisterPath(const QString &alias)
{
    // Make sure not to register twice to each connected client
    if (_registeredAliases.contains(alias))
        return;

    Folder *f = FolderMan::instance()->folder(alias);
    if (f) {
        QString message = buildRegisterPathMessage(removeTrailingSlash(f->path()));
        foreach (auto &listener, _listeners) {
            listener.sendMessage(message);
        }
    }

    _registeredAliases.insert(alias);
}

void SocketApi::slotUnregisterPath(const QString &alias)
{
    if (!_registeredAliases.contains(alias))
        return;

    Folder *f = FolderMan::instance()->folder(alias);
    if (f)
        broadcastMessage(buildMessage(QLatin1String("UNREGISTER_PATH"), removeTrailingSlash(f->path()), QString()), true);

    _registeredAliases.remove(alias);
}

void SocketApi::slotUpdateFolderView(Folder *f)
{
    if (_listeners.isEmpty()) {
        return;
    }

    if (f) {
        // do only send UPDATE_VIEW for a couple of status
        if (f->syncResult().status() == SyncResult::SyncPrepare
            || f->syncResult().status() == SyncResult::Success
            || f->syncResult().status() == SyncResult::Paused
            || f->syncResult().status() == SyncResult::Problem
            || f->syncResult().status() == SyncResult::Error
            || f->syncResult().status() == SyncResult::SetupError) {
            QString rootPath = removeTrailingSlash(f->path());
            broadcastStatusPushMessage(rootPath, f->syncEngine().syncFileStatusTracker().fileStatus(""));

            broadcastMessage(buildMessage(QLatin1String("UPDATE_VIEW"), rootPath));
        } else {
            qCDebug(lcSocketApi) << "Not sending UPDATE_VIEW for" << f->alias() << "because status() is" << f->syncResult().status();
        }
    }
}

void SocketApi::broadcastMessage(const QString &msg, bool doWait)
{
    foreach (auto &listener, _listeners) {
        listener.sendMessage(msg, doWait);
    }
}

void SocketApi::processShareRequest(const QString &localFile, SocketListener *listener, ShareDialogStartPage startPage)
{
    auto theme = Theme::instance();

    auto fileData = FileData::get(localFile);
    auto shareFolder = fileData.folder;
    if (!shareFolder) {
        const QString message = QLatin1String("SHARE:NOP:") + QDir::toNativeSeparators(localFile);
        // files that are not within a sync folder are not synced.
        listener->sendMessage(message);
    } else if (!shareFolder->accountState()->isConnected()) {
        const QString message = QLatin1String("SHARE:NOTCONNECTED:") + QDir::toNativeSeparators(localFile);
        // if the folder isn't connected, don't open the share dialog
        listener->sendMessage(message);
    } else if (!theme->linkSharing() && (!theme->userGroupSharing() || shareFolder->accountState()->account()->serverVersionInt() < Account::makeServerVersion(8, 2, 0))) {
        const QString message = QLatin1String("SHARE:NOP:") + QDir::toNativeSeparators(localFile);
        listener->sendMessage(message);
    } else {
        // If the file doesn't have a journal record, it might not be uploaded yet
        if (!fileData.journalRecord().isValid()) {
            const QString message = QLatin1String("SHARE:NOTSYNCED:") + QDir::toNativeSeparators(localFile);
            listener->sendMessage(message);
            return;
        }

        auto &remotePath = fileData.serverRelativePath;

        // Can't share root folder
        if (remotePath == "/") {
            const QString message = QLatin1String("SHARE:CANNOTSHAREROOT:") + QDir::toNativeSeparators(localFile);
            listener->sendMessage(message);
            return;
        }

        const QString message = QLatin1String("SHARE:OK:") + QDir::toNativeSeparators(localFile);
        listener->sendMessage(message);

        emit shareCommandReceived(remotePath, fileData.localPath, startPage);
    }
}

void SocketApi::broadcastStatusPushMessage(const QString &systemPath, SyncFileStatus fileStatus)
{
    QString msg = buildMessage(QLatin1String("STATUS"), systemPath, fileStatus.toSocketAPIString());
    Q_ASSERT(!systemPath.endsWith('/'));
    uint directoryHash = qHash(systemPath.left(systemPath.lastIndexOf('/')));
    foreach (auto &listener, _listeners) {
        listener.sendMessageIfDirectoryMonitored(msg, directoryHash);
    }
}

void SocketApi::command_RETRIEVE_FOLDER_STATUS(const QString &argument, SocketListener *listener)
{
    // This command is the same as RETRIEVE_FILE_STATUS
    command_RETRIEVE_FILE_STATUS(argument, listener);
}

void SocketApi::command_RETRIEVE_FILE_STATUS(const QString &argument, SocketListener *listener)
{
    QString statusString;

    auto fileData = FileData::get(argument);
    if (!fileData.folder) {
        // this can happen in offline mode e.g.: nothing to worry about
        statusString = QLatin1String("NOP");
    } else {
        // The user probably visited this directory in the file shell.
        // Let the listener know that it should now send status pushes for sibblings of this file.
        QString directory = fileData.localPath.left(fileData.localPath.lastIndexOf('/'));
        listener->registerMonitoredDirectory(qHash(directory));

        SyncFileStatus fileStatus = fileData.syncFileStatus();
        statusString = fileStatus.toSocketAPIString();
    }

    const QString message = QLatin1String("STATUS:") % statusString % QLatin1Char(':') % QDir::toNativeSeparators(argument);
    listener->sendMessage(message);
}

void SocketApi::command_SHARE(const QString &localFile, SocketListener *listener)
{
    processShareRequest(localFile, listener, ShareDialogStartPage::UsersAndGroups);
}

void SocketApi::command_MANAGE_PUBLIC_LINKS(const QString &localFile, SocketListener *listener)
{
    processShareRequest(localFile, listener, ShareDialogStartPage::PublicLinks);
}

void SocketApi::command_VERSION(const QString &, SocketListener *listener)
{
    listener->sendMessage(QLatin1String("VERSION:" MIRALL_VERSION_STRING ":" MIRALL_SOCKET_API_VERSION));
}

void SocketApi::command_SHARE_MENU_TITLE(const QString &, SocketListener *listener)
{
    listener->sendMessage(QLatin1String("SHARE_MENU_TITLE:") + tr("Share with %1", "parameter is ownCloud").arg(Theme::instance()->appNameGUI()));
}


void SocketApi::command_COPY_PUBLIC_LINK(const QString &localFile, SocketListener *)
{
    auto fileData = FileData::get(localFile);
    if (!fileData.folder)
        return;

    AccountPtr account = fileData.folder->accountState()->account();
    auto job = new GetOrCreatePublicLinkShare(account, fileData.serverRelativePath, this);
    connect(job, &GetOrCreatePublicLinkShare::done, this,
            [](const QString &url) { copyUrlToClipboard(url); });
    connect(job, &GetOrCreatePublicLinkShare::error, this,
            [=]() { emit shareCommandReceived(fileData.serverRelativePath, fileData.localPath, ShareDialogStartPage::PublicLinks); });
    job->run();
}

// Fetches the private link url asynchronously and then calls the target slot
void SocketApi::fetchPrivateLinkUrlHelper(const QString &localFile, const std::function<void(const QString &url)> &targetFun)
{
    auto fileData = FileData::get(localFile);
    if (!fileData.folder) {
        qCWarning(lcSocketApi) << "Unknown path" << localFile;
        return;
    }

    auto record = fileData.journalRecord();
    if (!record.isValid())
        return;

    fetchPrivateLinkUrl(
        fileData.folder->accountState()->account(),
        fileData.serverRelativePath,
        record.legacyDeriveNumericFileId(),
        this,
        targetFun);
}

void SocketApi::command_COPY_PRIVATE_LINK(const QString &localFile, SocketListener *)
{
    fetchPrivateLinkUrlHelper(localFile, &SocketApi::copyUrlToClipboard);
}

void SocketApi::command_EMAIL_PRIVATE_LINK(const QString &localFile, SocketListener *)
{
    fetchPrivateLinkUrlHelper(localFile, &SocketApi::emailPrivateLink);
}

void SocketApi::command_OPEN_PRIVATE_LINK(const QString &localFile, SocketListener *)
{
    fetchPrivateLinkUrlHelper(localFile, &SocketApi::openPrivateLink);
}

void SocketApi::command_OPEN_PRIVATE_LINK_VERSIONS(const QString &localFile, SocketListener *)
{
    auto openVersionsLink = [](const QString &link) {
        QUrl url(link);
        QUrlQuery query(url);
        query.addQueryItem(QStringLiteral("details"), QStringLiteral("versionsTabView"));
        url.setQuery(query);
        Utility::openBrowser(url, nullptr);
    };
    fetchPrivateLinkUrlHelper(localFile, openVersionsLink);
}

void SocketApi::copyUrlToClipboard(const QString &link)
{
    QApplication::clipboard()->setText(link);
}

void SocketApi::command_MAKE_AVAILABLE_LOCALLY(const QString &filesArg, SocketListener *)
{
    QStringList files = filesArg.split(QLatin1Char('\x1e')); // Record Separator

    if (files.count()) {
        for (const auto &file : files) {
            auto data = FileData::get(file);
            if (!data.folder)
                continue;

            // Avoid hydration by the sync engine
            /*OCC::SyncJournalFileRecord record;
            if (data.folder->journalDb()->getFileRecord(data.serverRelativePath, &record) && record.isValid()) {
                if (record._type != ItemTypeFile) {
                    record._type = ItemTypeFile;
                    data.folder->journalDb()->setFileRecord(record);
                }
            }*/

            QTemporaryFile *tmpFile = new QTemporaryFile();
            if (!tmpFile) {
                qCWarning(lcSocketApi) << "Unable to create temporary file!";
                continue;
            }
            tmpFile->open();

            QMap<QByteArray, QByteArray> headers;

            QPointer<GETFileJob> job = new GETFileJob(data.folder->accountState()->account(),
                data.serverRelativePath, tmpFile, headers, "", 0, this);
            job->setFolder(data.folder);
            connect(job.data(), &GETJob::finishedSignal, this, &SocketApi::slotGetFinished);
            connect(job.data(), &GETFileJob::writeProgress, this, &SocketApi::slotWriteProgress);
            job->start();
        }
    }
}

void SocketApi::slotWriteProgress(qint64 received)
{
    GETFileJob *job = qobject_cast<GETFileJob *>(sender());
    if (!job || !job->folder()) {
        qCWarning(lcSocketApi) << "Invalid job";
        return;
    }

    // Update VFS fetch status
    QTemporaryFile *tmpFile = static_cast<QTemporaryFile *>(job->device());
    if (!tmpFile) {
        qCWarning(lcSocketApi) << "Invalid temp file";
        job->reply()->abort();
        return;
    }
    tmpFile->flush();

    QString filePath = QFileInfo(job->folder()->path() + job->path()).canonicalFilePath();
    bool canceled = false;
    if (!job->folder()->vfs().updateFetchStatus(tmpFile->fileName(), filePath, received, canceled)) {
        qCWarning(lcSocketApi) << "Error in updateFetchStatus for file " << filePath;
        job->reply()->abort();
    }
    else if (canceled) {
        qCDebug(lcSocketApi) << "Update fetch status canceled for file " << job->path();
        job->reply()->abort();
    }
}

void SocketApi::slotGetFinished()
{
    GETFileJob *job = qobject_cast<GETFileJob *>(sender());
    if (!job || !job->folder()) {
        qCWarning(lcSocketApi) << "Invalid job";
        return;
    }

    if (job->device()) {
        job->device()->deleteLater();
    }
}

/* Go over all the files and replace them by a virtual file */
void SocketApi::command_MAKE_ONLINE_ONLY(const QString &filesArg, SocketListener *)
{
    QStringList files = filesArg.split(QLatin1Char('\x1e')); // Record Separator

    for (const auto &file : files) {
        auto data = FileData::get(file);
        if (!data.folder)
            continue;

        // Update the pin state on all items
        data.folder->vfs().setPinState(data.folderRelativePath, PinState::OnlineOnly);

        // Trigger sync
        data.folder->schedulePathForLocalDiscovery(data.folderRelativePath);
        data.folder->scheduleThisFolderSoon();
    }
}

void SocketApi::command_DELETE_ITEM(const QString &localFile, SocketListener *)
{
    QFileInfo info(localFile);

    auto result = QMessageBox::question(
        nullptr, tr("Confirm deletion"),
        info.isDir()
            ? tr("Do you want to delete the directory <i>%1</i> and all its contents permanently?").arg(info.dir().dirName())
            : tr("Do you want to delete the file <i>%1</i> permanently?").arg(info.fileName()),
        QMessageBox::Yes, QMessageBox::No);
    if (result != QMessageBox::Yes)
        return;

    if (info.isDir()) {
        FileSystem::removeRecursively(localFile);
    } else {
        QFile(localFile).remove();
    }
}

void SocketApi::command_MOVE_ITEM(const QString &localFile, SocketListener *)
{
    auto fileData = FileData::get(localFile);
    auto parentDir = fileData.parentFolder();
    if (!fileData.folder)
        return; // should not have shown menu item

    QString defaultDirAndName = fileData.folderRelativePath;

    // If it's a conflict, we want to save it under the base name by default
    if (Utility::isConflictFile(defaultDirAndName)) {
        defaultDirAndName = fileData.folder->journalDb()->conflictFileBaseName(fileData.folderRelativePath.toUtf8());
    }

    // If the parent doesn't accept new files, go to the root of the sync folder
    QFileInfo fileInfo(localFile);
    auto parentRecord = parentDir.journalRecord();
    if ((fileInfo.isFile() && !parentRecord._remotePerm.hasPermission(RemotePermissions::CanAddFile))
        || (fileInfo.isDir() && !parentRecord._remotePerm.hasPermission(RemotePermissions::CanAddSubDirectories))) {
        defaultDirAndName = QFileInfo(defaultDirAndName).fileName();
    }

    // Add back the folder path
    defaultDirAndName = QDir(fileData.folder->path()).filePath(defaultDirAndName);

    auto target = QFileDialog::getSaveFileName(
        nullptr,
        tr("Select new location..."),
        defaultDirAndName,
        QString(), nullptr, QFileDialog::HideNameFilterDetails);
    if (target.isEmpty())
        return;

    QString error;
    if (!FileSystem::uncheckedRenameReplace(localFile, target, &error)) {
        qCWarning(lcSocketApi) << "Rename error:" << error;
        QMessageBox::warning(
            nullptr, tr("Error"),
            tr("Moving file failed:\n\n%1").arg(error));
    }
}

void SocketApi::emailPrivateLink(const QString &link)
{
    Utility::openEmailComposer(
        tr("I shared something with you"),
        link,
        nullptr);
}

void OCC::SocketApi::openPrivateLink(const QString &link)
{
    Utility::openBrowser(link, nullptr);
}

void SocketApi::command_GET_STRINGS(const QString &argument, SocketListener *listener)
{
    static std::array<std::pair<const char *, QString>, 5> strings { {
        //{ "SHARE_MENU_TITLE", tr("Share...") },
        { "CONTEXT_MENU_TITLE", Theme::instance()->appNameGUI() },
        { "COPY_PRIVATE_LINK_MENU_TITLE", tr("Copy private link to clipboard") },
        { "EMAIL_PRIVATE_LINK_MENU_TITLE", tr("Send private link by email...") },
    } };
    listener->sendMessage(QString("GET_STRINGS:BEGIN"));
    for (auto key_value : strings) {
        if (argument.isEmpty() || argument == QLatin1String(key_value.first)) {
            listener->sendMessage(QString("STRING:%1:%2").arg(key_value.first, key_value.second));
        }
    }
    listener->sendMessage(QString("GET_STRINGS:END"));
}

void SocketApi::command_GET_THUMBNAIL(const QString &argument, OCC::SocketListener *listener)
{
    QStringList argumentList = argument.split(QLatin1Char('\x1e'));

    if (argumentList.size() != 3) {
        qCCritical(lcSocketApi) << "Invalid argument " << argument;
        return;
    }

    // Picture width asked
    unsigned int width(argumentList[0].toInt());
    if (width == 0) {
        qCCritical(lcSocketApi) << "Bad width " << width;
        return;
    }

    // File iNode
    uint64_t iNode(argumentList[1].toULongLong());

    // File path
    QString filePath(argumentList[2]);
    if (!QFileInfo(filePath).isFile()) {
        qCCritical(lcSocketApi) << "Not a file: " << filePath;
        return;
    }

    Folder *folder = FolderMan::instance()->folderForPath(filePath);
    if (!folder) {
        qCCritical(lcSocketApi) << "Folder not found for " << filePath;
        return;
    }

    QString fileRemotePath = folder->remotePathTrailingSlash() + filePath.midRef(folder->path().size()).toUtf8();
    if (fileRemotePath.startsWith('/')) {
        fileRemotePath.remove(0, 1);
    }
    ThumbnailJob *job = new ThumbnailJob(fileRemotePath, width, iNode, listener,
                                         folder->accountState()->account(), this);
    connect(job, &ThumbnailJob::jobFinished, this, &SocketApi::slotThumbnailFetched);
    job->start();
}

void SocketApi::sendSharingContextMenuOptions(const FileData &fileData, SocketListener *listener)
{
    auto record = fileData.journalRecord();
    bool isOnTheServer = record.isValid();
    auto flagString = isOnTheServer ? QLatin1String("::") : QLatin1String(":d:");

    auto capabilities = fileData.folder->accountState()->account()->capabilities();
    auto theme = Theme::instance();
    if (!capabilities.shareAPI() || !(theme->userGroupSharing() || (theme->linkSharing() && capabilities.sharePublicLink())))
        return;

    // If sharing is globally disabled, do not show any sharing entries.
    // If there is no permission to share for this file, add a disabled entry saying so
    if (isOnTheServer && !record._remotePerm.isNull() && !record._remotePerm.hasPermission(RemotePermissions::CanReshare)) {
        listener->sendMessage(QLatin1String("MENU_ITEM:DISABLED:d:") + (!record.isDirectory()
            ? tr("Resharing this file is not allowed") : tr("Resharing this folder is not allowed")));
    } else {
        //listener->sendMessage(QLatin1String("MENU_ITEM:SHARE") + flagString + tr("Share..."));

        // Do we have public links?
        bool publicLinksEnabled = theme->linkSharing() && capabilities.sharePublicLink();

        // Is is possible to create a public link without user choices?
        bool canCreateDefaultPublicLink = publicLinksEnabled
            && !capabilities.sharePublicLinkEnforcePassword();

        if (canCreateDefaultPublicLink) {
            listener->sendMessage(QLatin1String("MENU_ITEM:COPY_PUBLIC_LINK") + flagString + tr("Copy public link to clipboard"));
        } else if (publicLinksEnabled) {
            listener->sendMessage(QLatin1String("MENU_ITEM:MANAGE_PUBLIC_LINKS") + flagString + tr("Copy public link to clipboard"));
        }
    }

    listener->sendMessage(QLatin1String("MENU_ITEM:COPY_PRIVATE_LINK") + flagString + tr("Copy private link to clipboard"));

    // Disabled: only providing email option for private links would look odd,
    // and the copy option is more general.
    //listener->sendMessage(QLatin1String("MENU_ITEM:EMAIL_PRIVATE_LINK") + flagString + tr("Send private link by email..."));
}

SocketApi::FileData SocketApi::FileData::get(const QString &localFile)
{
    FileData data;

    data.localPath = QDir::cleanPath(localFile);
    if (data.localPath.endsWith(QLatin1Char('/')))
        data.localPath.chop(1);

    data.folder = FolderMan::instance()->folderForPath(data.localPath, &data.folderRelativePath);
    if (!data.folder)
        return data;

    data.serverRelativePath = QDir(data.folder->remotePath()).filePath(data.folderRelativePath);
    QString virtualFileExt = QStringLiteral(APPLICATION_DOTVIRTUALFILE_SUFFIX);
    if (data.serverRelativePath.endsWith(virtualFileExt)) {
        data.serverRelativePath.chop(virtualFileExt.size());
    }
    return data;
}

QString SocketApi::FileData::folderRelativePathNoVfsSuffix() const
{
    auto result = folderRelativePath;
    QString virtualFileExt = QStringLiteral(APPLICATION_DOTVIRTUALFILE_SUFFIX);
    if (result.endsWith(virtualFileExt)) {
        result.chop(virtualFileExt.size());
    }
    return result;
}

SyncFileStatus SocketApi::FileData::syncFileStatus() const
{
    if (!folder)
        return SyncFileStatus::StatusNone;
    return folder->syncEngine().syncFileStatusTracker().fileStatus(folderRelativePath);
}

SyncJournalFileRecord SocketApi::FileData::journalRecord() const
{
    SyncJournalFileRecord record;
    if (!folder)
        return record;
    folder->journalDb()->getFileRecord(folderRelativePath, &record);
    return record;
}

SocketApi::FileData SocketApi::FileData::parentFolder() const
{
    return FileData::get(QFileInfo(localPath).dir().path().toUtf8());
}

void SocketApi::command_GET_MENU_ITEMS(const QString &argument, OCC::SocketListener *listener)
{
    listener->sendMessage(QString("GET_MENU_ITEMS:BEGIN"));
    QStringList files = argument.split(QLatin1Char('\x1e')); // Record Separator

    // Find the common sync folder.
    // syncFolder will be null if files are in different folders.
    Folder *folder = nullptr;
    for (const auto &file : files) {
        auto f = FolderMan::instance()->folderForPath(file);
        if (f != folder) {
            if (!folder) {
                folder = f;
            } else {
                folder = nullptr;
                break;
            }
        }
    }

    // Some options only show for single files
    if (files.size() == 1) {
        FileData fileData = FileData::get(files.first());
        auto record = fileData.journalRecord();
        bool isOnTheServer = record.isValid();
        auto flagString = isOnTheServer ? QLatin1String("::") : QLatin1String(":d:");

        if (folder) {
            listener->sendMessage(QLatin1String("VFS_MODE:") + Vfs::modeToString(folder->vfs().mode()));
        }

        if (fileData.folder && fileData.folder->accountState()->isConnected()) {
            sendSharingContextMenuOptions(fileData, listener);
            listener->sendMessage(QLatin1String("MENU_ITEM:OPEN_PRIVATE_LINK") + flagString + tr("Open in browser"));

            // Add link to versions pane if possible
            auto &capabilities = folder->accountState()->account()->capabilities();
            if (capabilities.versioningEnabled()
                && capabilities.privateLinkDetailsParamAvailable()
                && isOnTheServer
                && !record.isDirectory()) {
                listener->sendMessage(QLatin1String("MENU_ITEM:OPEN_PRIVATE_LINK_VERSIONS") + flagString + tr("Show file versions in browser"));
            }

            // Conflict files get conflict resolution actions
            bool isConflict = Utility::isConflictFile(fileData.folderRelativePath);
            if (isConflict || !isOnTheServer) {
                // Check whether this new file is in a read-only directory
                QFileInfo fileInfo(fileData.localPath);
                auto parentDir = fileData.parentFolder();
                auto parentRecord = parentDir.journalRecord();
                bool canAddToDir =
                    (fileInfo.isFile() && !parentRecord._remotePerm.hasPermission(RemotePermissions::CanAddFile))
                    || (fileInfo.isDir() && !parentRecord._remotePerm.hasPermission(RemotePermissions::CanAddSubDirectories));
                bool canChangeFile =
                    !isOnTheServer
                    || (record._remotePerm.hasPermission(RemotePermissions::CanDelete)
                           && record._remotePerm.hasPermission(RemotePermissions::CanMove)
                           && record._remotePerm.hasPermission(RemotePermissions::CanRename));

                if (isConflict && canChangeFile) {
                    if (canAddToDir) {
                        if (isOnTheServer) {
                            // Conflict file that is already uploaded
                            listener->sendMessage(QLatin1String("MENU_ITEM:MOVE_ITEM::") + tr("Rename..."));
                        } else {
                            // Local-only conflict file
                            listener->sendMessage(QLatin1String("MENU_ITEM:MOVE_ITEM::") + tr("Rename and upload..."));
                        }
                    } else {
                        if (isOnTheServer) {
                            // Uploaded conflict file in read-only directory
                            listener->sendMessage(QLatin1String("MENU_ITEM:MOVE_ITEM::") + tr("Move and rename..."));
                        } else {
                            // Local-only conflict file in a read-only dir
                            listener->sendMessage(QLatin1String("MENU_ITEM:MOVE_ITEM::") + tr("Move, rename and upload..."));
                        }
                    }
                    listener->sendMessage(QLatin1String("MENU_ITEM:DELETE_ITEM::") + tr("Delete local changes"));
                }

                // File in a read-only directory?
                if (!isConflict && !isOnTheServer && !canAddToDir) {
                    listener->sendMessage(QLatin1String("MENU_ITEM:MOVE_ITEM::") + tr("Move and upload..."));
                    listener->sendMessage(QLatin1String("MENU_ITEM:DELETE_ITEM::") + tr("Delete"));
                }
            }
        }
    }

    // File availability actions
    if (folder
        && folder->supportsVirtualFiles()
        && folder->vfs().socketApiPinStateActionsShown()) {
        ENFORCE(!files.isEmpty());

        // Determine the combined availability status of the files
        auto combined = Optional<VfsItemAvailability>();
        auto merge = [](VfsItemAvailability lhs, VfsItemAvailability rhs) {
            if (lhs == rhs)
                return lhs;
            if (int(lhs) > int(rhs))
                std::swap(lhs, rhs); // reduce cases ensuring lhs < rhs
            if (lhs == VfsItemAvailability::AlwaysLocal && rhs == VfsItemAvailability::AllHydrated)
                return VfsItemAvailability::AllHydrated;
            if (lhs == VfsItemAvailability::AllDehydrated && rhs == VfsItemAvailability::OnlineOnly)
                return VfsItemAvailability::AllDehydrated;
            return VfsItemAvailability::Mixed;
        };
        for (const auto &file : files) {
            auto fileData = FileData::get(file);
            auto availability = folder->vfs().availability(fileData.folderRelativePath);
            if (!availability) {
                if (availability.error() == Vfs::AvailabilityError::DbError)
                    availability = VfsItemAvailability::Mixed;
                if (availability.error() == Vfs::AvailabilityError::NoSuchItem)
                    continue;
            }
            if (!combined) {
                combined = *availability;
            } else {
                combined = merge(*combined, *availability);
            }
        }

        // TODO: Should be a submenu, should use icons
        auto makePinContextMenu = [&](bool makeAvailableLocally, bool freeSpace) {
            listener->sendMessage(QLatin1String("MENU_ITEM:CURRENT_PIN:d:")
                                  + Utility::vfsCurrentAvailabilityText(*combined));
            listener->sendMessage(QLatin1String("MENU_ITEM:MAKE_AVAILABLE_LOCALLY:")
                                  + (makeAvailableLocally ? QLatin1String(":") : QLatin1String("d:"))
                                  + Utility::vfsPinActionText());
            listener->sendMessage(QLatin1String("MENU_ITEM:MAKE_ONLINE_ONLY:")
                                  + (freeSpace ? QLatin1String(":") : QLatin1String("d:"))
                                  + Utility::vfsFreeSpaceActionText());
        };

        if (combined) {
            switch (*combined) {
            case VfsItemAvailability::AlwaysLocal:
                makePinContextMenu(false, true);
                break;
            case VfsItemAvailability::AllHydrated:
            case VfsItemAvailability::Mixed:
                makePinContextMenu(true, true);
                break;
            case VfsItemAvailability::AllDehydrated:
            case VfsItemAvailability::OnlineOnly:
                makePinContextMenu(true, false);
                break;
            }
        }
    }

    listener->sendMessage(QString("GET_MENU_ITEMS:END"));
}

QString SocketApi::buildRegisterPathMessage(const QString &path)
{
    QFileInfo fi(path);
    QString message = QLatin1String("REGISTER_PATH:");
    message.append(QDir::toNativeSeparators(fi.absoluteFilePath()));
    return message;
}

} // namespace OCC

#include "socketapi.moc"
