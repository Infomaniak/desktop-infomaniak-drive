/*
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

#include "thumbnailjob.h"
#include "networkjobs.h"
#include "account.h"
#include "config.h"

#include <QDir>

namespace OCC {

ThumbnailJob::ThumbnailJob(const QString &fileRelativePath, unsigned int width, const OCC::SocketListener *listener,
                           AccountPtr account, QObject *parent)
    : AbstractNetworkJob(
          account,
          QString(APPLICATION_THUMBNAIL_URL).arg(width).arg(QDir::cleanPath(fileRelativePath)),
          parent)
    , _fileRelativePath(fileRelativePath)
    , _width(width)
    , _listener(listener)
{
    setIgnoreCredentialFailure(true);
}

void ThumbnailJob::start()
{
    sendRequest("GET", makeAccountUrl(path()));
    AbstractNetworkJob::start();
}

bool ThumbnailJob::finished()
{
    emit jobFinished(reply()->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), reply()->readAll(),
                     _fileRelativePath, _width, _listener);
    return true;
}
}
