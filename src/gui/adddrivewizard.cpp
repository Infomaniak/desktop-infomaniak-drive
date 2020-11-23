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

#include "adddrivewizard.h"
#include "custommessagebox.h"
#include "accountmanager.h"
#include "folderman.h"
#include "theme.h"
#include "creds/dummycredentials.h"
#include "clientproxy.h"
#include "configfile.h"
#include "filesystem.h"

#include <QBoxLayout>
#include <QDir>
#include <QNetworkProxy>

namespace KDC {

static const QSize windowSize(625, 700);
static const int boxHMargin = 40;
static const int boxVTMargin = 20;
static const int boxVBMargin = 40;

Q_LOGGING_CATEGORY(lcAddDriveWizard, "gui.adddrivewizard", QtInfoMsg)

AddDriveWizard::AddDriveWizard(QWidget *parent)
    : CustomDialog(parent)
    , _accountPtr(nullptr)
    , _stepStackedWidget(nullptr)
    , _addDriveStartWidget(nullptr)
    , _addDriveLoginWidget(nullptr)
    , _addDriveSmartSyncWidget(nullptr)
    , _addDriveServerFoldersWidget(nullptr)
    , _addDriveLocalFolderWidget(nullptr)
    , _addDriveConfirmationWidget(nullptr)
    , _currentStep(None)
    , _loginUrl(QString())
    , _serverFolderPath(QString())
    , _selectionSize(0)
    , _blackList(QStringList())
    , _serverUrl(QString())
    , _localFolderPath(QString())
    , _accountId(QString())
    , _action(OCC::Utility::WizardAction::OpenFolder)
{
    initUI();
    start();
}

void AddDriveWizard::setButtonIcon(const QColor &value)
{
    if (_addDriveSmartSyncWidget) {
        _addDriveSmartSyncWidget->setButtonIcon(value);
    }
    if (_addDriveServerFoldersWidget) {
        _addDriveServerFoldersWidget->setButtonIcon(value);
    }
    if (_addDriveLocalFolderWidget) {
        _addDriveLocalFolderWidget->setButtonIcon(value);
    }
}

void AddDriveWizard::initUI()
{
    setMinimumSize(windowSize);
    setMaximumSize(windowSize);

    QVBoxLayout *mainLayout = this->mainLayout();
    mainLayout->setContentsMargins(boxHMargin, boxVTMargin, boxHMargin, boxVBMargin);

    _stepStackedWidget = new QStackedWidget(this);
    mainLayout->addWidget(_stepStackedWidget);

    _addDriveStartWidget = new AddDriveStartWidget(true, this);
    _stepStackedWidget->insertWidget(Begin, _addDriveStartWidget);

    _addDriveLoginWidget = new AddDriveLoginWidget(this);
    _stepStackedWidget->insertWidget(Login, _addDriveLoginWidget);

    _addDriveSmartSyncWidget = new AddDriveSmartSyncWidget(this);
    _stepStackedWidget->insertWidget(RemoteFolders, _addDriveSmartSyncWidget);

    _addDriveServerFoldersWidget = new AddDriveServerFoldersWidget(this);
    _stepStackedWidget->insertWidget(RemoteFolders, _addDriveServerFoldersWidget);

    _addDriveLocalFolderWidget = new AddDriveLocalFolderWidget(this);
    _stepStackedWidget->insertWidget(LocalFolder, _addDriveLocalFolderWidget);

    _addDriveConfirmationWidget = new AddDriveConfirmationWidget(this);
    _stepStackedWidget->insertWidget(Confirmation, _addDriveConfirmationWidget);

    connect(_addDriveStartWidget, &AddDriveStartWidget::terminated, this, &AddDriveWizard::onStepTerminated);
    connect(_addDriveLoginWidget, &AddDriveLoginWidget::terminated, this, &AddDriveWizard::onStepTerminated);
    connect(_addDriveSmartSyncWidget, &AddDriveSmartSyncWidget::terminated, this, &AddDriveWizard::onStepTerminated);
    connect(_addDriveServerFoldersWidget, &AddDriveServerFoldersWidget::terminated, this, &AddDriveWizard::onStepTerminated);
    connect(_addDriveLocalFolderWidget, &AddDriveLocalFolderWidget::terminated, this, &AddDriveWizard::onStepTerminated);
    connect(_addDriveConfirmationWidget, &AddDriveConfirmationWidget::terminated, this, &AddDriveWizard::onStepTerminated);
    connect(this, &CustomDialog::exit, this, &AddDriveWizard::onExit);
}

void AddDriveWizard::start()
{
    OCC::FolderMan::instance()->setSyncEnabled(false);

    _accountPtr = OCC::AccountManager::createAccount();
    _accountPtr->setCredentials(new OCC::DummyCredentials);
    _accountPtr->setUrl(OCC::Theme::instance()->overrideServerUrl());
    _accountId = _accountPtr->id();

    startNextStep();
}

void AddDriveWizard::startNextStep(bool backward)
{
    _currentStep = (Step) (_currentStep + (backward ? -1 : 1));

    if (_currentStep == SmartSync
            && OCC::bestAvailableVfsMode(OCC::ConfigFile().showExperimentalOptions()) != OCC::Vfs::WindowsCfApi) {
        // Skip Lite Sync step
        _currentStep = (Step) (_currentStep + (backward ? -1 : 1));
    }
    else if (_currentStep == RemoteFolders && _smartSync) {
        // Skip Remote Folders step
        _currentStep = (Step) (_currentStep + (backward ? -1 : 1));
    }

    _stepStackedWidget->setCurrentIndex(_currentStep);

    if (_currentStep == Begin) {
        setBackgroundForcedColor(QColor());
        _addDriveStartWidget->setAccountPtr(_accountPtr);
        _addDriveStartWidget->setServerUrl(_accountPtr->url().toString());
    }
    else if (_currentStep == Login) {
        setBackgroundForcedColor(Qt::white);
        _serverUrl = _addDriveStartWidget->serverUrl();
        _addDriveLoginWidget->setAccountPtr(_accountPtr);
        _addDriveLoginWidget->login(_serverUrl);
    }
    else if (_currentStep == SmartSync) {
        setBackgroundForcedColor(QColor());
    }
    else if (_currentStep == RemoteFolders) {
        setBackgroundForcedColor(QColor());
        _addDriveServerFoldersWidget->setAccountPtr(_accountPtr);
    }
    else if (_currentStep == LocalFolder) {
        setBackgroundForcedColor(QColor());
        _addDriveLocalFolderWidget->setAccountPtr(_accountPtr);
        _addDriveLocalFolderWidget->setSmartSync(_smartSync);
        QString localFolderPath = OCC::Theme::instance()->defaultClientFolder();
        if (!QDir(localFolderPath).isAbsolute()) {
            localFolderPath = QDir::homePath() + dirSeparator + localFolderPath;
        }
        QString goodLocalFolderPath = OCC::FolderMan::instance()->findGoodPathForNewSyncFolder(localFolderPath, _serverUrl);
        _addDriveLocalFolderWidget->setLocalFolderPath(goodLocalFolderPath);
    }
    else if (_currentStep == Confirmation) {
        setBackgroundForcedColor(QColor());
        _addDriveConfirmationWidget->setFolderPath(_localFolderPath);
    }
}

void AddDriveWizard::checkServer(const QString &urlString)
{
    QString fixedUrl = urlString;
    QUrl url = QUrl::fromUserInput(fixedUrl);
    // fromUserInput defaults to http, not http if no scheme is specified
    if (!fixedUrl.startsWith("http://") && !fixedUrl.startsWith("https://")) {
        url.setScheme("https");
    }
    _accountPtr->setUrl(url);

    // Reset the proxy which might had been determined previously in ConnectionValidator::checkServerAndAuth()
    // when there was a previous account.
    _accountPtr->networkAccessManager()->setProxy(QNetworkProxy(QNetworkProxy::NoProxy));

    // And also reset the QSslConfiguration, for the same reason (#6832)
    // Here the client certificate is added, if any. Later it'll be in HttpCredentials
    _accountPtr->setSslConfiguration(QSslConfiguration());
    auto sslConfiguration = _accountPtr->getOrCreateSslConfig(); // let Account set defaults
    _accountPtr->setSslConfiguration(sslConfiguration);

    // Make sure TCP connections get re-established
    _accountPtr->networkAccessManager()->clearAccessCache();

    // Lookup system proxy in a thread https://github.com/owncloud/client/issues/2993
    if (OCC::ClientProxy::isUsingSystemDefault()) {
        qCDebug(lcAddDriveWizard) << "Trying to look up system proxy";
        OCC::ClientProxy::lookupSystemProxyAsync(_accountPtr->url(), this, SLOT(onSystemProxyLookupDone(QNetworkProxy)));
    } else {
        // We want to reset the QNAM proxy so that the global proxy settings are used (via ClientProxy settings)
        _accountPtr->networkAccessManager()->setProxy(QNetworkProxy(QNetworkProxy::DefaultProxy));
        // use a queued invocation so we're as asynchronous as with the other code path
        QMetaObject::invokeMethod(this, "onFindServer", Qt::QueuedConnection);
    }
}

void AddDriveWizard::setCredentials(OCC::AbstractCredentials *creds)
{
    _accountPtr->setCredentials(creds);
    testConnection();

    // Fetch user information
    OCC::ConnectionValidator *conValidator = new OCC::ConnectionValidator(_accountPtr);
    conValidator->fetchUser();
}

QString AddDriveWizard::printQNetworkProxy(const QNetworkProxy &proxy)
{
    return QString("%1://%2:%3").arg(proxy.type()).arg(proxy.hostName()).arg(proxy.port());
}

void AddDriveWizard::setAuthType(OCC::DetermineAuthTypeJob::AuthType type)
{
    if (type == OCC::DetermineAuthTypeJob::WebViewFlow) {
        startNextStep();
    }
    else {
        qCDebug(lcAddDriveWizard) << "Authentication type not managed";
    }
}

bool AddDriveWizard::checkDowngradeAdvised(QNetworkReply *reply)
{
    if (reply->url().scheme() != QLatin1String("https")) {
        return false;
    }

    switch (reply->error()) {
    case QNetworkReply::NoError:
    case QNetworkReply::ContentNotFoundError:
    case QNetworkReply::AuthenticationRequiredError:
    case QNetworkReply::HostNotFoundError:
        return false;
    default:
        break;
    }

    // Adhere to HSTS, even though we do not parse it properly
    if (reply->hasRawHeader("Strict-Transport-Security")) {
        return false;
    }
    return true;
}

void AddDriveWizard::testConnection()
{
    auto *job = new OCC::PropfindJob(_accountPtr, "/", this);
    job->setIgnoreCredentialFailure(true);
    // There is custom redirect handling in the error handler,
    // so don't automatically follow redirects.
    job->setFollowRedirects(false);
    job->setProperties(QList<QByteArray>() << "getlastmodified");
    connect(job, &OCC::PropfindJob::result, this, &AddDriveWizard::onAuthTestOk);
    connect(job, &OCC::PropfindJob::finishedWithError, this, &AddDriveWizard::onAuthTestError);
    job->start();
}

OCC::AccountState *AddDriveWizard::applyAccountChanges()
{
    OCC::AccountPtr newAccountPtr = _accountPtr;

    // Detach the account that is going to be saved from the wizard to ensure it doesn't accidentally get modified later
    _accountPtr = OCC::AccountManager::createAccount();

    auto manager = OCC::AccountManager::instance();
    auto newAccountState = manager->addAccount(newAccountPtr);
    manager->save();
    return newAccountState;
}

bool AddDriveWizard::addDrive()
{
    bool startFromScratch = false;

    auto accountState = applyAccountChanges();

    QString localFolderPath = OCC::FolderDefinition::prepareLocalPath(_localFolderPath);
    const QDir localFolderDir(localFolderPath);
    if (localFolderDir.exists()) {
        OCC::FileSystem::setFolderMinimumPermissions(localFolderPath);
        OCC::Utility::setupFavLink(localFolderPath);
    } else {
        QString res = tr("Creating local sync folder %1...").arg(localFolderPath);
        if (localFolderDir.mkpath(localFolderPath)) {
            OCC::FileSystem::setFolderMinimumPermissions(localFolderPath);
            OCC::Utility::setupFavLink(localFolderPath);
        } else {
            qCWarning(lcAddDriveWizard) << "Failed to create " << localFolderDir.path();
            CustomMessageBox *msgBox = new CustomMessageBox(
                        QMessageBox::Warning,
                        tr("Failed to create local folder %1").arg(localFolderDir.path()),
                        QMessageBox::Ok, this);
            msgBox->exec();
            return false;
        }
    }

    if (!startFromScratch) {
        qCInfo(lcAddDriveWizard) << "Adding folder definition for" << localFolderPath << _serverFolderPath;

        OCC::FolderDefinition folderDefinition;
        folderDefinition.localPath = localFolderPath;
        folderDefinition.targetPath = OCC::FolderDefinition::prepareTargetPath(_serverFolderPath);
        folderDefinition.ignoreHiddenFiles = OCC::FolderMan::instance()->ignoreHiddenFiles();
        if (_smartSync) {
            folderDefinition.virtualFilesMode = OCC::bestAvailableVfsMode(OCC::ConfigFile().showExperimentalOptions());
        }
        if (OCC::FolderMan::instance()->navigationPaneHelper().showInExplorerNavigationPane()) {
            folderDefinition.navigationPaneClsid = QUuid::createUuid();
        }

        OCC::Folder *folder = OCC::FolderMan::instance()->addFolder(accountState, folderDefinition);
        if (folder) {
            if (folderDefinition.virtualFilesMode != OCC::Vfs::Off && _smartSync) {
                folder->setRootPinState(OCC::PinState::OnlineOnly);
            }

            folder->journalDb()->setSelectiveSyncList(OCC::SyncJournalDb::SelectiveSyncBlackList, _blackList);
            OCC::ConfigFile cfg;
            if (!cfg.newBigFolderSizeLimit().first) {
                folder->journalDb()->setSelectiveSyncList(OCC::SyncJournalDb::SelectiveSyncWhiteList, QStringList() << dirSeparator);
            }
        }
    }
    return true;
}

void AddDriveWizard::onSystemProxyLookupDone(const QNetworkProxy &proxy)
{
    if (proxy.type() != QNetworkProxy::NoProxy) {
        qCInfo(lcAddDriveWizard) << "Setting QNAM proxy to be system proxy" << printQNetworkProxy(proxy);
    } else {
        qCInfo(lcAddDriveWizard) << "No system proxy set by OS";
    }
    _accountPtr->networkAccessManager()->setProxy(proxy);

    onFindServer();
}

void AddDriveWizard::onFindServer()
{
    // Set fake credentials before we check what credential it actually is.
    _accountPtr->setCredentials(new OCC::DummyCredentials);

    // Determining the actual server URL can be a multi-stage process
    // 1. Check url/status.php with CheckServerJob
    //    If that works we're done. In that case we don't check the
    //    url directly for redirects, see #5954.
    // 2. Check the url for permanent redirects (like url shorteners)
    // 3. Check redirected-url/status.php with CheckServerJob

    // Step 1: Check url/status.php
    OCC::CheckServerJob *job = new OCC::CheckServerJob(_accountPtr, this);
    job->setIgnoreCredentialFailure(true);
    connect(job, &OCC::CheckServerJob::instanceFound, this, &AddDriveWizard::onFoundServer);
    connect(job, &OCC::CheckServerJob::instanceNotFound, this, &AddDriveWizard::onFindServerBehindRedirect);
    connect(job, &OCC::CheckServerJob::timeout, this, &AddDriveWizard::onNoServerFoundTimeout);
    job->setTimeout((_accountPtr->url().scheme() == "https") ? 30 * 1000 : 10 * 1000);
    job->start();

    // Step 2 and 3 are in slotFindServerBehindRedirect()
}

void AddDriveWizard::onFindServerBehindRedirect()
{
    // Step 2: Resolve any permanent redirect chains on the base url
    auto redirectCheckJob = _accountPtr->sendRequest("GET", _accountPtr->url());

    // Use a significantly reduced timeout for this redirect check:
    // the 5-minute default is inappropriate.
    redirectCheckJob->setTimeout(qMin(2000ll, redirectCheckJob->timeoutMsec()));

    // Grab the chain of permanent redirects and adjust the account url
    // accordingly
    auto permanentRedirects = std::make_shared<int>(0);
    connect(redirectCheckJob, &OCC::AbstractNetworkJob::redirected, this,
        [=](QNetworkReply *reply, const QUrl &targetUrl, int count) {
            int httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (count == *permanentRedirects && (httpCode == 301 || httpCode == 308)) {
                qCInfo(lcAddDriveWizard) << _accountPtr->url() << " was redirected to" << targetUrl;
                _accountPtr->setUrl(targetUrl);
                *permanentRedirects += 1;
            }
        });

    // Step 3: When done, start checking status.php.
    connect(redirectCheckJob, &OCC::SimpleNetworkJob::finishedSignal, this,
        [=]() {
            OCC::CheckServerJob *job = new OCC::CheckServerJob(_accountPtr, this);
            job->setIgnoreCredentialFailure(true);
            connect(job, &OCC::CheckServerJob::instanceFound, this, &AddDriveWizard::onFoundServer);
            connect(job, &OCC::CheckServerJob::instanceNotFound, this, &AddDriveWizard::onNoServerFound);
            connect(job, &OCC::CheckServerJob::timeout, this, &AddDriveWizard::onNoServerFoundTimeout);
            job->setTimeout((_accountPtr->url().scheme() == "https") ? 30 * 1000 : 10 * 1000);
            job->start();
    });
}

void AddDriveWizard::onFoundServer(const QUrl &url, const QJsonObject &info)
{
    auto serverVersion = OCC::CheckServerJob::version(info);

    // Note with newer servers we get the version actually only later in capabilities
    // https://github.com/owncloud/core/pull/27473/files
    _accountPtr->setServerVersion(serverVersion);

    if (url != _accountPtr->url()) {
        // We might be redirected, update the account
        _accountPtr->setUrl(url);
        qCInfo(lcAddDriveWizard) << " was redirected to" << url.toString();
    }

    onDetermineAuthType();
}

void AddDriveWizard::onNoServerFound(QNetworkReply *reply)
{
    Q_UNUSED(reply)

    auto job = qobject_cast<OCC::CheckServerJob *>(sender());

    // Do this early because reply might be deleted in message box event loop
    QString msg;
    if (!_accountPtr->url().isValid()) {
        msg = tr("Invalid URL");
    } else {
        msg = tr("Failed to connect to %1 at %2:<br/>%3")
                  .arg(OCC::Utility::escape(OCC::Theme::instance()->appNameGUI()),
                      OCC::Utility::escape(_accountPtr->url().toString()),
                      OCC::Utility::escape(job->errorString()));
    }
    //bool isDowngradeAdvised = checkDowngradeAdvised(reply);

    // Displays message inside wizard and possibly also another message box
    CustomMessageBox *msgBox = new CustomMessageBox(
                QMessageBox::Warning,
                msg,
                QMessageBox::Ok, this);
    msgBox->exec();

    // Allow the credentials dialog to pop up again for the same URL.
    // Maybe the user just clicked 'Cancel' by accident or changed his mind.
    _accountPtr->resetRejectedCertificates();
}

void AddDriveWizard::onNoServerFoundTimeout(const QUrl &url)
{
    CustomMessageBox *msgBox = new CustomMessageBox(
                QMessageBox::Warning,
                tr("Timeout while trying to connect to %1 at %2.")
                .arg(OCC::Utility::escape(OCC::Theme::instance()->appNameGUI()), OCC::Utility::escape(url.toString())),
                QMessageBox::Ok, this);
    msgBox->exec();
}

void AddDriveWizard::onDetermineAuthType()
{
    OCC::DetermineAuthTypeJob *job = new OCC::DetermineAuthTypeJob(_accountPtr, this);
    connect(job, &OCC::DetermineAuthTypeJob::authType, this, &AddDriveWizard::setAuthType);
    job->start();
}

void AddDriveWizard::onAuthTestOk()
{
    startNextStep();
}

void AddDriveWizard::onAuthTestError()
{
    QString errorMsg;

    OCC::PropfindJob *job = qobject_cast<OCC::PropfindJob *>(sender());
    if (!job) {
        qCWarning(lcAddDriveWizard)  << "Can't check for authed redirects. This slot should be invoked from PropfindJob!";
        return;
    }
    QNetworkReply *reply = job->reply();

    // If there were redirects on the *authed* requests, also store
    // the updated server URL, similar to redirects on status.php.
    QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (!redirectUrl.isEmpty()) {
        qCInfo(lcAddDriveWizard) << "Authed request was redirected to" << redirectUrl.toString();

        // strip the expected path
        QString path = redirectUrl.path();
        static QString expectedPath = dirSeparator + _accountPtr->davPath();
        if (path.endsWith(expectedPath)) {
            path.chop(expectedPath.size());
            redirectUrl.setPath(path);

            qCInfo(lcAddDriveWizard) << "Setting account url to" << redirectUrl.toString();
            _accountPtr->setUrl(redirectUrl);
            testConnection();
            return;
        }
        errorMsg = tr("The authenticated request to the server was redirected to "
                      "'%1'. The URL is bad, the server is misconfigured.")
                       .arg(OCC::Utility::escape(redirectUrl.toString()));

        // A 404 is actually a success: we were authorized to know that the folder does
        // not exist. It will be created later...
    } else if (reply->error() == QNetworkReply::ContentNotFoundError) {
        onAuthTestOk();
        return;

        // Provide messages for other errors, such as invalid credentials.
    } else if (reply->error() != QNetworkReply::NoError) {
        if (!_accountPtr->credentials()->stillValid(reply)) {
            errorMsg = tr("Access forbidden by server. To verify that you have proper access, "
                          "<a href=\"%1\">click here</a> to access the service with your browser.")
                           .arg(OCC::Utility::escape(_accountPtr->url().toString()));
        } else {
            errorMsg = job->errorStringParsingBody();
        }

        // Something else went wrong, maybe the response was 200 but with invalid data.
    } else {
        errorMsg = tr("There was an invalid response to an authenticated webdav request");
    }

    show();
    CustomMessageBox *msgBox = new CustomMessageBox(
                QMessageBox::Critical,
                errorMsg,
                QMessageBox::Ok, this);
    msgBox->exec();
}

void AddDriveWizard::onStepTerminated(bool next)
{
    if (_currentStep == Begin) {
        checkServer(_addDriveStartWidget->serverUrl());
    }
    else if (_currentStep == Login) {
        _loginUrl = _addDriveLoginWidget->loginUrl();
        setCredentials(_addDriveLoginWidget->credentials());
    }
    else if (_currentStep == SmartSync) {
        if (next) {
            _smartSync = _addDriveSmartSyncWidget->smartSync();
        }
        startNextStep(!next);
    }
    else if (_currentStep == RemoteFolders) {
        if (next) {
            _selectionSize = _addDriveServerFoldersWidget->selectionSize();
            _blackList = _addDriveServerFoldersWidget->createBlackList();
        }
        startNextStep(!next);
    }
    else if (_currentStep == LocalFolder) {
        if (next) {
            _localFolderPath = _addDriveLocalFolderWidget->localFolderPath();
            if (_smartSync) {
                _smartSync = _addDriveLocalFolderWidget->folderCompatibleWithSmartSync();
            }
            if (!addDrive()) {
                reject();
            }
        }
        startNextStep(!next);
    }
    else if (_currentStep == Confirmation) {
        _action = _addDriveConfirmationWidget->action();
        accept();
    }
}

void AddDriveWizard::onExit()
{
    reject();
}

}

