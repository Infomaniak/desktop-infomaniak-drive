/*
Infomaniak Drive
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

#include "customdialog.h"
#include "adddrivestartwidget.h"
#include "adddriveloginwidget.h"
#include "adddriveserverfolderswidget.h"
#include "adddrivelocalfolderwidget.h"
#include "adddriveconfirmationwidget.h"
#include "networkjobs.h"

#include <QStackedWidget>

namespace KDC {

class AddDriveWizard : public CustomDialog
{
    Q_OBJECT

public:
    explicit AddDriveWizard(QWidget *parent = nullptr);

    inline OCC::Utility::WizardAction nextAction() { return _action; }
    inline QString localFolderPath() { return _localFolderPath; }
    inline QString accountId() { return _accountPtr->id(); }

private:
    enum Step {
        None = -1,
        Begin,
        Login,
        RemoteFoders,
        LocalFolder,
        Confirmation
    };

    OCC::AccountPtr _accountPtr;
    QStackedWidget *_stepStackedWidget;
    AddDriveStartWidget *_addDriveStartWidget;
    AddDriveLoginWidget *_addDriveLoginWidget;
    AddDriveServerFoldersWidget *_addDriveServerFoldersWidget;
    AddDriveLocalFolderWidget *_addDriveLocalFolderWidget;
    AddDriveConfirmationWidget *_addDriveConfirmationWidget;
    Step _currentStep;
    QString _loginUrl;
    QString _serverFolderPath;
    qint64 _selectionSize;
    QStringList _blackList;
    QString _serverUrl;
    QString _localFolderPath;
    OCC::Utility::WizardAction _action;

    void setButtonIcon(const QColor &value) override;

    void initUI();
    void start();
    void startNextStep();
    void checkServer(const QString &urlString);
    void setCredentials(OCC::AbstractCredentials *creds);
    QString printQNetworkProxy(const QNetworkProxy &proxy);
    void setAuthType(OCC::DetermineAuthTypeJob::AuthType type);
    bool checkDowngradeAdvised(QNetworkReply *reply);
    void testConnection();
    OCC::AccountState *applyAccountChanges();
    void addDrive();

private slots:
    void onStepTerminated(bool next = true);
    void onExit();
    void onSystemProxyLookupDone(const QNetworkProxy &proxy);
    void onFindServer();
    void onFindServerBehindRedirect();
    void onFoundServer(const QUrl &url, const QJsonObject &info);
    void onNoServerFound(QNetworkReply *reply);
    void onNoServerFoundTimeout(const QUrl &url);
    void onDetermineAuthType();
    void onAuthTestOk();
    void onAuthTestError();
};

}

