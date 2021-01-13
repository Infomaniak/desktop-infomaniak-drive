#pragma once

#include "account.h"
#ifndef OWNCLOUD_TEST
#include "sharemanager.h"
#endif

#include <QList>
#include <QObject>
#include <QSharedPointer>
#include <QString>

namespace OCC {

// don't pull the share manager into socketapi unittests
#ifndef OWNCLOUD_TEST

class GetOrCreatePublicLinkShare : public QObject
{
    Q_OBJECT
public:
    GetOrCreatePublicLinkShare(const AccountPtr &account, const QString &serverPath, QObject *parent);

    void run();

private slots:
    void sharesFetched(const QList<QSharedPointer<Share>> &shares);
    void linkShareCreated(const QSharedPointer<LinkShare> &share);
    void linkShareCreationForbidden(const QString &message);
    void serverError(int code, const QString &message);

signals:
    void done(const QString &link);
    void error(const QString &message);

private:
    void success(const QString &link);

    AccountPtr _account;
    ShareManager _shareManager;
    QString _serverPath;
};

#else

class GetOrCreatePublicLinkShare : public QObject
{
    Q_OBJECT
public:
    GetOrCreatePublicLinkShare(const AccountPtr &, const QString &, QObject *)
    {
    }

    void run()
    {
    }
signals:
    void done(const QString &link);
    void error(int code, const QString &message);
};

#endif

}
