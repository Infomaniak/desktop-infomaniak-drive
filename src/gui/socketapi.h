/*
 * Copyright (C) by Dominik Schmidt <dev@dominik-schmidt.de>
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


#ifndef SOCKETAPI_H
#define SOCKETAPI_H

#include "syncfileitem.h"
#include "common/syncfilestatus.h"
#include "sharedialog.h" // for the ShareDialogStartPage
#include "common/syncjournalfilerecord.h"
#include "socketlistener.h"
#include "propagatedownload.h"

#if defined(Q_OS_MAC)
#include "socketapisocket_mac.h"
#else
#include <QLocalServer>
typedef QLocalServer SocketApiServer;
#endif

#ifdef Q_OS_WIN
#include <deque>
#include <unordered_map>

#include <QList>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#endif

#define WORKER_GETFILE 0
#define NB_WORKERS 1

class QUrl;
class QLocalSocket;
class QStringList;

namespace OCC {

class SyncFileStatus;
class Folder;

#ifdef Q_OS_WIN
struct GetFileJobInfo
{
    GetFileJobInfo() = default;
    GetFileJobInfo(QNetworkReply *reply, Vfs *vfs, QTemporaryFile *tmpFile, qint64 received)
        : _reply(reply)
        , _vfs(vfs)
        , _tmpFile(tmpFile)
        , _received(received)
    {}

    QNetworkReply *_reply;
    Vfs *_vfs;
    QTemporaryFile *_tmpFile;
    qint64 _received;
    bool _toProcess = true;
    bool _processing = false;
    bool _toFinish = false;
};

struct WorkerInfo
{
    QMutex _mutex;
    std::deque<QString> _queue;
    QWaitCondition _queueWC;
    bool _stop = false;
    QWaitCondition _stopWC;
    int _nbRunningThreads = 0;
    QList<QThread *> _threadList;
};
#endif

// Helper structure for getting information on a file
// based on its local path - used for nearly all remote
// actions.
struct FileData
{
    static FileData get(const QString &localFile);
    SyncFileStatus syncFileStatus() const;
    SyncJournalFileRecord journalRecord() const;
    FileData parentFolder() const;

    // Relative path of the file locally, without any vfs suffix
    QString folderRelativePathNoVfsSuffix() const;

    Folder *folder;
    // Absolute path of the file locally. (May be a virtual file)
    QString localPath;
    // Relative path of the file locally, as in the DB. (May be a virtual file)
    QString folderRelativePath;
    // Path of the file on the server (In case of virtual file, it points to the actual file)
    QString serverRelativePath;
};

/**
 * @brief The SocketApi class
 * @ingroup gui
 */
class SocketApi : public QObject
{
    Q_OBJECT

public:
#ifdef Q_OS_WIN
    WorkerInfo _workerInfo[NB_WORKERS];

    std::unordered_map<QString, GetFileJobInfo> _getFileJobMap;
    QMutex _getFileJobMapMutex;
#endif

    explicit SocketApi(QObject *parent = 0);
    virtual ~SocketApi();

    static QString getJobFilePath(const GETJob *job);

public slots:
    void slotUpdateFolderView(Folder *f);
    void slotUnregisterPath(const QString &alias);
    void slotRegisterPath(const QString &alias);
    void broadcastStatusPushMessage(const QString &systemPath, SyncFileStatus fileStatus);

signals:
    void shareCommandReceived(const QString &sharePath, const QString &localPath, ShareDialogStartPage startPage);

private slots:
    void slotNewConnection();
    void onLostConnection();
    void slotSocketDestroyed(QObject *obj);
    void slotReadSocket();
    void slotThumbnailFetched(const int &statusCode, const QByteArray &reply,
                              unsigned int width, uint64_t iNode, const OCC::SocketListener *listener);

    void slotWriteProgress(qint64 received);
    void slotGetFinished();

    static void copyUrlToClipboard(const QString &link);
    static void emailPrivateLink(const QString &link);
    static void openPrivateLink(const QString &link);

private:
    void broadcastMessage(const QString &msg, bool doWait = false);

    // opens share dialog, sends reply
    void processShareRequest(const QString &localFile, SocketListener *listener, ShareDialogStartPage startPage);

    Q_INVOKABLE void command_RETRIEVE_FOLDER_STATUS(const QString &argument, SocketListener *listener);
    Q_INVOKABLE void command_RETRIEVE_FILE_STATUS(const QString &argument, SocketListener *listener);

    Q_INVOKABLE void command_VERSION(const QString &argument, SocketListener *listener);

    Q_INVOKABLE void command_SHARE_MENU_TITLE(const QString &argument, SocketListener *listener);

    // The context menu actions
    Q_INVOKABLE void command_SHARE(const QString &localFile, SocketListener *listener);
    Q_INVOKABLE void command_MANAGE_PUBLIC_LINKS(const QString &localFile, SocketListener *listener);
    Q_INVOKABLE void command_COPY_PUBLIC_LINK(const QString &localFile, SocketListener *listener);
    Q_INVOKABLE void command_COPY_PRIVATE_LINK(const QString &localFile, SocketListener *listener);
    Q_INVOKABLE void command_EMAIL_PRIVATE_LINK(const QString &localFile, SocketListener *listener);
    Q_INVOKABLE void command_OPEN_PRIVATE_LINK(const QString &localFile, SocketListener *listener);
    Q_INVOKABLE void command_OPEN_PRIVATE_LINK_VERSIONS(const QString &localFile, SocketListener *listener);
    Q_INVOKABLE void command_MAKE_AVAILABLE_LOCALLY(const QString &filesArg, SocketListener *listener);
#ifdef Q_OS_WIN
    Q_INVOKABLE void command_MAKE_AVAILABLE_LOCALLY_DIRECT(const QString &filesArg, SocketListener *listener);
#endif
    Q_INVOKABLE void command_MAKE_ONLINE_ONLY(const QString &filesArg, SocketListener *listener);
    Q_INVOKABLE void command_DELETE_ITEM(const QString &localFile, SocketListener *listener);
    Q_INVOKABLE void command_MOVE_ITEM(const QString &localFile, SocketListener *listener);

    // Fetch the private link and call targetFun
    void fetchPrivateLinkUrlHelper(const QString &localFile, const std::function<void(const QString &url)> &targetFun);

    /** Sends translated/branded strings that may be useful to the integration */
    Q_INVOKABLE void command_GET_STRINGS(const QString &argument, SocketListener *listener);

    /** Sends the request URL to get a thumbnail */
    Q_INVOKABLE void command_GET_THUMBNAIL(const QString &localFile, SocketListener *listener);

    // Sends the context menu options relating to sharing to listener
    void sendSharingContextMenuOptions(const FileData &argument, SocketListener *listener);

    /** Send the list of menu item. (added in version 1.1)
     * argument is a list of files for which the menu should be shown, separated by '\x1e'
     * Reply with  GET_MENU_ITEMS:BEGIN
     * followed by several MENU_ITEM:[Action]:[flag]:[Text]
     * If flag contains 'd', the menu should be disabled
     * and ends with GET_MENU_ITEMS:END
     */
    Q_INVOKABLE void command_GET_MENU_ITEMS(const QString &argument, SocketListener *listener);

    QString buildRegisterPathMessage(const QString &path);

    QSet<QString> _registeredAliases;
    QList<SocketListener> _listeners;
    SocketApiServer _localServer;
};

class Worker : public QObject
{
    Q_OBJECT

public:
    Worker(SocketApi *socketApi, int type, int num);
    void start();

private:
    SocketApi *_socketApi;
    int _type;
    int _num;
};

}
#endif // SOCKETAPI_H
