/*
Copyright (C) 2021 christophe.larchier@infomaniak.com

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

#include "accountinfo.h"
#include "account.h"

#include <QFile>

namespace KDC {

Q_LOGGING_CATEGORY(lcAccountInfo, "gui.accountinfo", QtInfoMsg)

AccountInfo::AccountInfo()
    : _name(QString())
    , _color(QColor())
    , _isSignedIn(false)
    , _paused(false)
    , _unresolvedConflicts(false)
    , _status(OCC::SyncResult::Status::Undefined)
    , _folderMap(FolderMap())
    , _quotaInfoPtr(nullptr)
    , _totalSize(0)
    , _used(0)
    , _errorsCount(0)
{
}

AccountInfo::AccountInfo(OCC::AccountState *accountState)
    : AccountInfo()
{
    initQuotaInfo(accountState);
}

void AccountInfo::initQuotaInfo(OCC::AccountState *accountState)
{
    if (accountState) {
        _quotaInfoPtr = std::unique_ptr<OCC::QuotaInfo>(new OCC::QuotaInfo(accountState));
        _quotaInfoPtr.get()->setActive(false);
        _quotaInfoPtr.get()->setProperty(accountIdProperty, accountState->account()->id());
    }
}

void AccountInfo::updateStatus()
{
    _status = OCC::SyncResult::Undefined;
    _paused = false;
    _unresolvedConflicts = false;

    std::size_t cnt = _folderMap.size();

    if (cnt == 1) {
        FolderInfo *folderInfo = _folderMap.begin()->second.get();
        if (folderInfo) {
            if (folderInfo->_paused) {
                _status = OCC::SyncResult::Paused;
                _paused = true;
            } else {
                switch (folderInfo->_status) {
                case OCC::SyncResult::Undefined:
                    _status = OCC::SyncResult::Error;
                    break;
                case OCC::SyncResult::Problem: // don't show the problem
                    _status = OCC::SyncResult::Success;
                    break;
                default:
                    _status = folderInfo->_status;
                    break;
                }
            }
            _unresolvedConflicts = folderInfo->_unresolvedConflicts;
        }
        else {
            qCDebug(lcAccountInfo) << "Null pointer!";
            Q_ASSERT(false);
        }
    } else {
        int errorsSeen = 0;
        int goodSeen = 0;
        int abortOrPausedSeen = 0;
        int runSeen = 0;

        for (auto const &folderMapElt : _folderMap) {
            const FolderInfo *folderInfo = folderMapElt.second.get();
            if (folderInfo) {
                if (folderInfo->_paused) {
                    abortOrPausedSeen++;
                } else {
                    switch (folderInfo->_status) {
                    case OCC::SyncResult::Undefined:
                        break;
                    case OCC::SyncResult::NotYetStarted:
                    case OCC::SyncResult::SyncPrepare:
                    case OCC::SyncResult::SyncRunning:
                        runSeen++;
                        break;
                    case OCC::SyncResult::Problem: // don't show the problem
                    case OCC::SyncResult::Success:
                        goodSeen++;
                        break;
                    case OCC::SyncResult::Error:
                    case OCC::SyncResult::SetupError:
                        errorsSeen++;
                        break;
                    case OCC::SyncResult::SyncAbortRequested:
                    case OCC::SyncResult::Paused:
                        abortOrPausedSeen++;
                    }
                }
                if (folderInfo->_unresolvedConflicts) {
                    _unresolvedConflicts = true;
                }
            }
            else {
                qCDebug(lcAccountInfo) << "Null pointer!";
                Q_ASSERT(false);
            }
        }

        if (errorsSeen > 0) {
            _status = OCC::SyncResult::Error;
        } else if (abortOrPausedSeen > 0 && abortOrPausedSeen == int(cnt)) {
            // All folders are paused
            _status = OCC::SyncResult::Paused;
            _paused = true;
        } else if (runSeen > 0) {
            _status = OCC::SyncResult::SyncRunning;
        } else if (goodSeen > 0) {
            _status = OCC::SyncResult::Success;
        }
    }
}

QString AccountInfo::folderPath(const QString &folderId, const QString &filePath) const
{
    QString fullFilePath = QString();

    const auto folderInfoIt = _folderMap.find(folderId);
    if (folderInfoIt != _folderMap.end()) {
        if (folderInfoIt->second) {
            fullFilePath = folderInfoIt->second->_path + filePath;
        }
        else {
            qCDebug(lcAccountInfo) << "Null pointer!";
            Q_ASSERT(false);
        }
    }

    return fullFilePath;
}

}
