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

#include "syncresult.h"

#include <QString>

namespace KDC {

struct FolderInfo {
    QString _name;
    QString _path;
    bool _paused;
    bool _unresolvedConflicts;
    OCC::SyncResult::Status _status;
    qint64 _currentFile;
    qint64 _totalFiles;
    qint64 _completedSize;
    qint64 _totalSize;
    qint64 _estimatedRemainingTime;

    FolderInfo(const QString &name = QString(), const QString &path = QString());
};

}

