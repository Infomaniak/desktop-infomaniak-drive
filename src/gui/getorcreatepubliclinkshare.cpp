#include "getorcreatepubliclinkshare.h"

namespace OCC {

Q_LOGGING_CATEGORY(lcPublicLink, "gui.publiclink", QtInfoMsg)

OCC::GetOrCreatePublicLinkShare::GetOrCreatePublicLinkShare(const OCC::AccountPtr &account, const QString &serverPath, QObject *parent)
    : QObject(parent)
    , _account(account)
    , _shareManager(account)
    , _serverPath(serverPath)
{
    connect(&_shareManager, &ShareManager::sharesFetched, this, &GetOrCreatePublicLinkShare::sharesFetched);
    connect(&_shareManager, &ShareManager::linkShareCreated, this, &GetOrCreatePublicLinkShare::linkShareCreated);
    connect(&_shareManager, &ShareManager::linkShareCreationForbidden, this, &GetOrCreatePublicLinkShare::linkShareCreationForbidden);
    connect(&_shareManager, &ShareManager::serverError, this, &GetOrCreatePublicLinkShare::serverError);
}

void OCC::GetOrCreatePublicLinkShare::run()
{
    qCDebug(lcPublicLink) << "Fetching shares";
    _shareManager.fetchShares(_serverPath);
}

void OCC::GetOrCreatePublicLinkShare::sharesFetched(const QList<QSharedPointer<OCC::Share> > &shares)
{
    auto shareName = tr("Context menu share");

    // If shares will expire, create a new one every day.
    QDate expireDate;
    if (_account->capabilities().sharePublicLinkDefaultExpire()) {
        shareName = tr("Context menu share %1").arg(QDate::currentDate().toString(Qt::ISODate));
        expireDate = QDate::currentDate().addDays(_account->capabilities().sharePublicLinkDefaultExpireDateDays());
    }

    // If there already is a context menu share, reuse it
    for (const auto &share : shares) {
        const auto linkShare = qSharedPointerDynamicCast<LinkShare>(share);
        if (!linkShare)
            continue;

        if (linkShare->getName() == shareName) {
            qCDebug(lcPublicLink) << "Found existing share, reusing";
            return success(linkShare->getLink().toString());
        }
    }

    // otherwise create a new one
    qCDebug(lcPublicLink) << "Creating new share";
    QString noPassword;
    _shareManager.createLinkShare(_serverPath, shareName, noPassword, expireDate);
}

void OCC::GetOrCreatePublicLinkShare::linkShareCreated(const QSharedPointer<OCC::LinkShare> &share)
{
    qCDebug(lcPublicLink) << "New share created";
    success(share->getLink().toString());
}

void OCC::GetOrCreatePublicLinkShare::linkShareCreationForbidden(const QString &message)
{
    qCInfo(lcPublicLink) << "Could not create link share:" << message;
    emit error(message);
    deleteLater();
}

void OCC::GetOrCreatePublicLinkShare::serverError(int code, const QString &message)
{
    qCWarning(lcPublicLink) << "Share fetch/create error" << code << message;
    emit error(message);
    deleteLater();
}

void OCC::GetOrCreatePublicLinkShare::success(const QString &link)
{
    emit done(link);
    deleteLater();
}

}
