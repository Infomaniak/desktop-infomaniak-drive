/*
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

#pragma once

#include "folderinfo.h"
#include "accountstate.h"
#include "quotainfo.h"

#include <QColor>

namespace KDC {

static const char accountIdProperty[] = "accountId";

typedef std::map<QString, std::shared_ptr<FolderInfo>> FolderMap;

struct AccountInfo {
    QString _name;
    QColor _color;
    bool _isSignedIn;
    bool _paused;
    bool _unresolvedConflicts;
    OCC::SyncResult::Status _status;
    FolderMap _folderMap;
    std::shared_ptr<OCC::QuotaInfo> _quotaInfoPtr;
    qint64 _totalSize;
    qint64 _used;
    int _errorsCount;

    AccountInfo();
    AccountInfo(OCC::AccountState *accountState);
    void initQuotaInfo(OCC::AccountState *accountState);
    void updateStatus();
    QString folderPath(const QString &folderId, const QString &filePath) const;
};

}

