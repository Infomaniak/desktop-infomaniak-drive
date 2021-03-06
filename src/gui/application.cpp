/*
 * Copyright (C) by Duncan Mac-Vicar P. <duncan@kde.org>
 * Copyright (C) by Klaas Freitag <freitag@owncloud.com>
 * Copyright (C) by Daniel Molkentin <danimo@owncloud.com>
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

#include "application.h"
#include "guiutility.h"

#include <iostream>
#include <random>

#include "config.h"
#include "common/asserts.h"
#include "account.h"
#include "accountstate.h"
#include "connectionvalidator.h"
#include "folder.h"
#include "folderman.h"
#include "logger.h"
#include "configfile.h"
#include "socketapi.h"
#include "sslerrordialog.h"
#include "theme.h"
#include "clientproxy.h"
#include "sharedialog.h"
#include "accountmanager.h"
#include "creds/abstractcredentials.h"
#include "updater/ocupdater.h"
#include "adddrivewizard.h"
#include "version.h"
#include "csync_exclude.h"
#include "common/vfs.h"
#include "libcommon/commonutility.h"
#include "customproxystyle.h"

#include "config.h"

#if defined(Q_OS_WIN)
#include <windows.h>
#endif

#if defined(WITH_CRASHREPORTER)
#include <libcrashreporter-handler/Handler.h>
#endif

#include <QTranslator>
#include <QMenu>
#include <QMessageBox>
#include <QDesktopServices>
#include <QFontDatabase>

class QSocket;

namespace OCC {

Q_LOGGING_CATEGORY(lcApplication, "gui.application", QtInfoMsg)

namespace {

    static const char optionsC[] =
        "Options:\n"
        "  -h --help            : show this help screen.\n"
        "  --settings           : show the Settings window (if the application is running).\n"
        "  --synthesis          : show the Synthesis window (if the application is running).\n"
        "  --logfile <filename> : write log output to file <filename>.\n"
        "  --logfile -          : write log output to stdout.\n"
        "  --logdir <name>      : write each sync log output in a new file\n"
        "                         in folder <name>.\n"
        "  --logexpire <hours>  : removes logs older than <hours> hours.\n"
        "                         (to be used with --logdir)\n"
        "  --logflush           : flush the log file after every write.\n"
        "  --logdebug           : also output debug-level messages in the log.\n"
        "  --confdir <dirname>  : Use the given configuration folder.\n";
}

static const QList<QString> fontFiles =
        QList<QString>()
        << QString(":/client/resources/fonts/SuisseIntl-Thin.otf")
        << QString(":/client/resources/fonts/SuisseIntl-UltraLight.otf")
        << QString(":/client/resources/fonts/SuisseIntl-Light.otf")
        << QString(":/client/resources/fonts/SuisseIntl-Regular.otf")
        << QString(":/client/resources/fonts/SuisseIntl-Medium.otf")
        << QString(":/client/resources/fonts/SuisseIntl-SemiBold.otf")
        << QString(":/client/resources/fonts/SuisseIntl-Bold.otf")
        << QString(":/client/resources/fonts/SuisseIntl-Black.otf");
//static const QString defaultFontFamily("Suisse Int'l");

// ----------------------------------------------------------------------------------

bool Application::configVersionMigration()
{
    QStringList deleteKeys, ignoreKeys;
    AccountManager::backwardMigrationSettingsKeys(&deleteKeys, &ignoreKeys);
    FolderMan::backwardMigrationSettingsKeys(&deleteKeys, &ignoreKeys);

    ConfigFile configFile;

    // Did the client version change?
    // (The client version is adjusted further down)
    bool versionChanged = configFile.clientVersionString() != MIRALL_VERSION_STRING;

    // We want to message the user either for destructive changes,
    // or if we're ignoring something and the client version changed.
    bool warningMessage = !deleteKeys.isEmpty() || (!ignoreKeys.isEmpty() && versionChanged);

    if (!versionChanged && !warningMessage)
        return true;

    const auto backupFile = configFile.backup();

    if (warningMessage) {
        QString boldMessage;
        if (!deleteKeys.isEmpty()) {
            boldMessage = tr("Continuing will mean <b>deleting these settings</b>.");
        } else {
            boldMessage = tr("Continuing will mean <b>ignoring these settings</b>.");
        }

        QMessageBox box(
            QMessageBox::Warning,
            APPLICATION_SHORTNAME,
            tr("Some settings were configured in newer versions of this client and "
               "use features that are not available in this version.<br>"
               "<br>"
               "%1<br>"
               "<br>"
               "The current configuration file was already backed up to <i>%2</i>.")
                .arg(boldMessage, backupFile));
        box.addButton(tr("Quit"), QMessageBox::AcceptRole);
        auto continueBtn = box.addButton(tr("Continue"), QMessageBox::DestructiveRole);

        box.exec();
        if (box.clickedButton() != continueBtn) {
            QTimer::singleShot(0, qApp, SLOT(quit()));
            return false;
        }

        auto settings = ConfigFile::settingsWithGroup("foo");
        settings->endGroup();

        // Wipe confusing keys from the future, ignore the others
        for (const auto &badKey : deleteKeys)
            settings->remove(badKey);
    }

    configFile.setClientVersionString(MIRALL_VERSION_STRING);
    return true;
}

Application::Application(int &argc, char **argv)
    : SharedTools::QtSingleApplication(Theme::instance()->appName(), argc, argv)
    , _gui(0)
    , _theme(Theme::instance())
    , _helpOnly(false)
    , _versionOnly(false)
    , _logExpire(0)
    , _logFlush(false)
    , _logDebug(false)
    , _userTriggeredConnect(false)
    , _debugMode(false)
    , _alert(false)
{
    _startedAt.start();

    qsrand(std::random_device()());

#ifdef Q_OS_WIN
    // Ensure OpenSSL config file is only loaded from app directory
    QString opensslConf = QCoreApplication::applicationDirPath() + QString("/openssl.cnf");
    qputenv("OPENSSL_CONF", opensslConf.toLocal8Bit());
#endif

    // TODO: Can't set this without breaking current config paths
    //    setOrganizationName(QLatin1String(APPLICATION_VENDOR));
    setOrganizationDomain(QLatin1String(APPLICATION_REV_DOMAIN));
    setApplicationName(_theme->appName());
    setWindowIcon(_theme->applicationIcon());
    setApplicationVersion(_theme->version());
    setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    setStyle(new KDC::CustomProxyStyle);

    if (!ConfigFile().exists()) {
        // Migrate from version <= 2.4
        setApplicationName(_theme->appNameGUI());
#ifndef QT_WARNING_DISABLE_DEPRECATED // Was added in Qt 5.9
#define QT_WARNING_DISABLE_DEPRECATED QT_WARNING_DISABLE_GCC("-Wdeprecated-declarations")
#endif
        QT_WARNING_PUSH
        QT_WARNING_DISABLE_DEPRECATED
        // We need to use the deprecated QDesktopServices::storageLocation because of its Qt4
        // behavior of adding "data" to the path
        QString oldDir = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
        if (oldDir.endsWith('/')) oldDir.chop(1); // macOS 10.11.x does not like trailing slash for rename/move.
        QT_WARNING_POP
        setApplicationName(_theme->appName());
        if (QFileInfo(oldDir).isDir()) {
            auto confDir = ConfigFile().configPath();
            if (confDir.endsWith('/')) confDir.chop(1);  // macOS 10.11.x does not like trailing slash for rename/move.
            qCInfo(lcApplication) << "Migrating old config from" << oldDir << "to" << confDir;

            if (!QFile::rename(oldDir, confDir)) {
                qCWarning(lcApplication) << "Failed to move the old config directory to its new location (" << oldDir << "to" << confDir << ")";

                // Try to move the files one by one
                if (QFileInfo(confDir).isDir() || QDir().mkdir(confDir)) {
                    const QStringList filesList = QDir(oldDir).entryList(QDir::Files);
                    qCInfo(lcApplication) << "Will move the individual files" << filesList;
                    for (const auto &name : filesList) {
                        if (!QFile::rename(oldDir + "/" + name,  confDir + "/" + name)) {
                            qCWarning(lcApplication) << "Fallback move of " << name << "also failed";
                        }
                    }
                }
            } else {
#ifndef Q_OS_WIN
                // Create a symbolic link so a downgrade of the client would still find the config.
                QFile::link(confDir, oldDir);
#endif
            }
        }
    }

    parseOptions(arguments());
    //no need to waste time;
    if (_helpOnly || _versionOnly)
        return;

    if (isRunning())
        return;

    setupLogging();

#if defined(WITH_CRASHREPORTER)
    if (ConfigFile().crashReporter())
        _crashHandler.reset(new CrashReporter::Handler(QDir::tempPath(), true, CRASHREPORTER_EXECUTABLE));
#endif

    CommonUtility::setupTranslations(this, Theme::instance()->enforcedLocale());

    if (!configVersionMigration()) {
        return;
    }

    ConfigFile cfg;
    // The timeout is initialized with an environment variable, if not, override with the value from the config
    if (!AbstractNetworkJob::httpTimeout)
        AbstractNetworkJob::httpTimeout = cfg.timeout();

#ifdef PLUGINDIR
    // Setup extra plugin search path
    QString extraPluginPath = QStringLiteral(PLUGINDIR);
    if (!extraPluginPath.isEmpty()) {
        if (QDir::isRelativePath(extraPluginPath))
            extraPluginPath = QDir(QApplication::applicationDirPath()).filePath(extraPluginPath);
        qCInfo(lcApplication) << "Adding extra plugin search path:" << extraPluginPath;
        addLibraryPath(extraPluginPath);
    }
#endif

    // Check vfs plugins
    if (isVfsPluginAvailable(Vfs::WindowsCfApi))
        qCInfo(lcApplication) << "VFS windows plugin is available";
    if (isVfsPluginAvailable(Vfs::WithSuffix))
        qCInfo(lcApplication) << "VFS suffix plugin is available";

    _folderManager.reset(new FolderMan);

    connect(this, &SharedTools::QtSingleApplication::messageReceived, this, &Application::slotParseMessage);

    if (!AccountManager::instance()->restore()) {
        // If there is an error reading the account settings, try again
        // after a couple of seconds, if that fails, give up.
        // (non-existence is not an error)
        Utility::sleep(5);
        if (!AccountManager::instance()->restore()) {
            qCCritical(lcApplication) << "Could not read the account settings, quitting";
            QMessageBox::critical(
                0,
                tr("Error accessing the configuration file"),
                tr("There was an error while accessing the configuration "
                   "file at %1.")
                    .arg(ConfigFile().configFile()),
                tr("Quit ownCloud"));
            QTimer::singleShot(0, qApp, SLOT(quit()));
            return;
        }
    }

    FolderMan::instance()->setSyncEnabled(true);

    setQuitOnLastWindowClosed(false);

    _theme->setSystrayUseMonoIcons(cfg.monoIcons());
    connect(_theme, &Theme::systrayUseMonoIconsChanged, this, &Application::slotUseMonoIconsChanged);

    // Setting up the gui class will allow tray notifications for the
    // setup that follows, like folder setup
    _gui = new OwnCloudGui(this);

    FolderMan::instance()->setupFolders();
    _proxy.setupQtProxyFromConfig(); // folders have to be defined first, than we set up the Qt proxy.

    // Load fonts
    for (QString fontFile : fontFiles) {
        if (QFontDatabase::addApplicationFont(fontFile) < 0) {
            qCInfo(lcApplication) << "Error adding font file!";
        }
    }

    // Set default font
    /*QFont font(defaultFontFamily);
    font.setStyleStrategy(QFont::PreferAntialias);
    QApplication::setFont(font);*/

    // Set style
    Utility::setStyle(qApp);

    connect(AccountManager::instance(), &AccountManager::accountAdded,
        this, &Application::slotAccountStateAdded);
    connect(AccountManager::instance(), &AccountManager::accountRemoved,
        this, &Application::slotAccountStateRemoved);
    foreach (auto ai, AccountManager::instance()->accounts()) {
        slotAccountStateAdded(ai.data());
    }

    connect(FolderMan::instance()->socketApi(), &SocketApi::shareCommandReceived,
        _gui.data(), &OwnCloudGui::slotShowShareDialog);

    // startup procedure.
    connect(&_checkConnectionTimer, &QTimer::timeout, this, &Application::slotCheckConnection);
    _checkConnectionTimer.setInterval(ConnectionValidator::DefaultCallingIntervalMsec); // check for connection every 32 seconds.
    _checkConnectionTimer.start();
    // Also check immediately
    QTimer::singleShot(0, this, &Application::slotCheckConnection);

    // Can't use onlineStateChanged because it is always true on modern systems because of many interfaces
    connect(&_networkConfigurationManager, &QNetworkConfigurationManager::configurationChanged,
        this, &Application::slotSystemOnlineConfigurationChanged);

    // Update checks
    UpdaterScheduler *updaterScheduler = new UpdaterScheduler(this);
    connect(updaterScheduler, &UpdaterScheduler::updaterAnnouncement,
        _gui.data(), &OwnCloudGui::slotShowTrayMessage);
    connect(updaterScheduler, &UpdaterScheduler::requestRestart,
        _folderManager.data(), &FolderMan::slotScheduleAppRestart);

    // Cleanup at Quit.
    connect(this, &QCoreApplication::aboutToQuit, this, &Application::slotCleanup);

#ifdef Q_OS_WIN
    // Update shortcuts
    FolderMan::instance()->navigationPaneHelper().setShowInExplorerNavigationPane(false);
    if (cfg.showInExplorerNavigationPane()) {
        FolderMan::instance()->navigationPaneHelper().setShowInExplorerNavigationPane(true);
    }
#endif
}

Application::~Application()
{
    // Make sure all folders are gone, otherwise removing the
    // accounts will remove the associated folders from the settings.
    if (_folderManager) {
        _folderManager->unloadAndDeleteAllFolders();
    }

    // Remove the account from the account manager so it can be deleted.
    AccountManager::instance()->shutdown();
}

void Application::showSynthesisDialog()
{
    _gui->showSynthesisDialog();
}

void Application::slotAccountStateRemoved(AccountState *accountState)
{
    if (_gui) {
        disconnect(accountState, &AccountState::stateChanged,
            _gui.data(), &OwnCloudGui::slotAccountStateChanged);
        disconnect(accountState->account().data(), &Account::serverVersionChanged,
            _gui.data(), &OwnCloudGui::slotTrayMessageIfServerUnsupported);
    }
    if (_folderManager) {
        disconnect(accountState, &AccountState::stateChanged,
            _folderManager.data(), &FolderMan::slotAccountStateChanged);
        disconnect(accountState->account().data(), &Account::serverVersionChanged,
            _folderManager.data(), &FolderMan::slotServerVersionChanged);
    }

    // if there is no more account, show the wizard.
    if (AccountManager::instance()->accounts().isEmpty()) {
        // allow to add a new account if there is non any more. Always think
        // about single account theming!
        //OwncloudSetupWizard::runWizard(this, SLOT(slotownCloudWizardDone(int)));
        /*KDC::AddDriveWizard *wizard = new KDC::AddDriveWizard();
        Application *app = qobject_cast<Application *>(qApp);
        app->slotownCloudWizardDone(wizard->exec());*/
    }
}

void Application::slotAccountStateAdded(AccountState *accountState)
{
    connect(accountState, &AccountState::stateChanged,
        _gui.data(), &OwnCloudGui::slotAccountStateChanged);
    connect(accountState->account().data(), &Account::serverVersionChanged,
        _gui.data(), &OwnCloudGui::slotTrayMessageIfServerUnsupported);
    connect(accountState, &AccountState::stateChanged,
        _folderManager.data(), &FolderMan::slotAccountStateChanged);
    connect(accountState->account().data(), &Account::serverVersionChanged,
        _folderManager.data(), &FolderMan::slotServerVersionChanged);

    _gui->slotTrayMessageIfServerUnsupported(accountState->account().data());
}

void Application::slotCleanup()
{
    AccountManager::instance()->save();
    FolderMan::instance()->unloadAndDeleteAllFolders();

    _gui->slotShutdown();
    _gui->deleteLater();
}

// FIXME: This is not ideal yet since a ConnectionValidator might already be running and is in
// progress of timing out in some seconds.
// Maybe we need 2 validators, one triggered by timer, one by network configuration changes?
void Application::slotSystemOnlineConfigurationChanged(QNetworkConfiguration cnf)
{
    if (cnf.state() & QNetworkConfiguration::Active) {
        QMetaObject::invokeMethod(this, "slotCheckConnection", Qt::QueuedConnection);
    }
}

void Application::slotCheckConnection()
{
    auto list = AccountManager::instance()->accounts();
    foreach (const auto &accountState, list) {
        AccountState::State state = accountState->state();

        // Don't check if we're manually signed out or
        // when the error is permanent.
        if (state != AccountState::SignedOut
            && state != AccountState::ConfigurationError
            && state != AccountState::AskingCredentials) {
            accountState->checkConnectivity();
        }
    }

    if (list.isEmpty()) {
        // let gui open the setup wizard
        _gui->slotOpenParametersDialog();
    }
}

void Application::slotCrash()
{
    Utility::crash();
}

void Application::slotCrashEnforce()
{
    ENFORCE(1==0);
}


void Application::slotCrashFatal()
{
    qFatal("la Qt fatale");
}


void Application::slotownCloudWizardDone(int res)
{
    AccountManager *accountMan = AccountManager::instance();
    FolderMan *folderMan = FolderMan::instance();

    // During the wizard, scheduling of new syncs is disabled
    folderMan->setSyncEnabled(true);

    if (res == QDialog::Accepted) {
        // Check connectivity of the newly created account
        _checkConnectionTimer.start();
        slotCheckConnection();

        // If one account is configured: enable autostart
        bool shouldSetAutoStart = (accountMan->accounts().size() == 1);
#ifdef Q_OS_MAC
        // Don't auto start when not being 'installed'
        shouldSetAutoStart = shouldSetAutoStart
            && QCoreApplication::applicationDirPath().startsWith("/Applications/");
#endif
        if (shouldSetAutoStart) {
            Utility::setLaunchOnStartup(_theme->appName(), _theme->appNameGUI(), true);
        }
    }
}

void Application::setupLogging()
{
    // might be called from second instance
    auto logger = Logger::instance();
    logger->setLogFile(_logFile);
    logger->setLogDir(_logDir);
    logger->setLogExpire(_logExpire);
    logger->setLogFlush(_logFlush);
    logger->setLogDebug(_logDebug);
    logger->enterNextLogFile();

    ConfigFile config;
    logger->setMinLogLevel(config.minLogLevel());

    if (config.automaticLogDir()) {
        // Don't override other configured logging
        if (logger->isLoggingToFile())
            return;

        logger->setupTemporaryFolderLogDir();
        if (auto deleteOldLogsHours = config.automaticDeleteOldLogsAge()) {
            logger->setLogExpire(*deleteOldLogsHours);
        } else {
            logger->setLogExpire(std::chrono::hours(0));
        }
        logger->enterNextLogFile();
    } else {
        logger->disableTemporaryFolderLogDir();
    }

    qCInfo(lcApplication) << QString::fromLatin1("################## %1 locale:[%2] ui_lang:[%3] version:[%4] os:[%5]").arg(_theme->appName()).arg(QLocale::system().name()).arg(property("ui_lang").toString()).arg(_theme->version()).arg(Utility::platformName());
}

void Application::slotUseMonoIconsChanged(bool)
{
    _gui->slotComputeOverallSyncStatus();
}

void Application::slotParseMessage(const QString &msg, QObject *)
{
    if (msg.startsWith(QLatin1String("MSG_PARSEOPTIONS:"))) {
        const int lengthOfMsgPrefix = 17;
        QStringList options = msg.mid(lengthOfMsgPrefix).split(QLatin1Char('|'));
        parseOptions(options);
        setupLogging();
    } else if (msg.startsWith(QLatin1String("MSG_SHOWSETTINGS"))) {
        qCInfo(lcApplication) << "Running for" << _startedAt.elapsed() / 1000.0 << "sec";
        if (_startedAt.elapsed() < 10 * 1000) {
            // This call is mirrored with the one in int main()
            qCWarning(lcApplication) << "Ignoring MSG_SHOWSETTINGS, possibly double-invocation of client via session restore and auto start";
            return;
        }
        showSettingsDialog();
    }
    else if (msg.startsWith(QLatin1String("MSG_SHOWSYNTHESIS"))) {
        qCInfo(lcApplication) << "Running for" << _startedAt.elapsed() / 1000.0 << "sec";
        if (_startedAt.elapsed() < 10 * 1000) {
            // This call is mirrored with the one in int main()
            qCWarning(lcApplication) << "Ignoring MSG_SHOWSETTINGS, possibly double-invocation of client via session restore and auto start";
            return;
        }

        showSynthesisDialog();
    }
}

void Application::parseOptions(const QStringList &options)
{
    QStringListIterator it(options);
    // skip file name;
    if (it.hasNext())
        it.next();

    //parse options; if help or bad option exit
    while (it.hasNext()) {
        QString option = it.next();
        if (option == QLatin1String("--help") || option == QLatin1String("-h")) {
            setHelp();
            break;
        } else if (option == QLatin1String("--logfile")) {
            if (it.hasNext() && !it.peekNext().startsWith(QLatin1String("--"))) {
                _logFile = it.next();
            } else {
                showHint("Log file not specified");
            }
        } else if (option == QLatin1String("--logdir")) {
            if (it.hasNext() && !it.peekNext().startsWith(QLatin1String("--"))) {
                _logDir = it.next();
            } else {
                showHint("Log dir not specified");
            }
        } else if (option == QLatin1String("--logexpire")) {
            if (it.hasNext() && !it.peekNext().startsWith(QLatin1String("--"))) {
                _logExpire = std::chrono::hours(it.next().toInt());
            } else {
                showHint("Log expiration not specified");
            }
        } else if (option == QLatin1String("--logflush")) {
            _logFlush = true;
        } else if (option == QLatin1String("--logdebug")) {
            _logDebug = true;
        } else if (option == QLatin1String("--confdir")) {
            if (it.hasNext() && !it.peekNext().startsWith(QLatin1String("--"))) {
                QString confDir = it.next();
                if (!ConfigFile::setConfDir(confDir)) {
                    showHint("Invalid path passed to --confdir");
                }
            } else {
                showHint("Path for confdir not specified");
            }
        } else if (option == QLatin1String("--debug")) {
            _logDebug = true;
            _debugMode = true;
        } else if (option == QLatin1String("--version")) {
            _versionOnly = true;
        } else if (option.endsWith(QStringLiteral(APPLICATION_DOTVIRTUALFILE_SUFFIX))) {
            // virtual file, open it after the Folder were created (if the app is not terminated)
            QTimer::singleShot(0, this, [this, option] { openVirtualFile(option); });
        } else if (option == QLatin1String("--settings")) {
        } else if (option == QLatin1String("--synthesis")) {
        } else {
            showHint("Unrecognized option '" + option.toStdString() + "'");
        }
    }
}

// Helpers for displaying messages. Note that there is no console on Windows.
#ifdef Q_OS_WIN
static void displayHelpText(const QString &t) // No console on Windows.
{
    QString spaces(80, ' '); // Add a line of non-wrapped space to make the messagebox wide enough.
    QString text = QLatin1String("<qt><pre style='white-space:pre-wrap'>")
        + t.toHtmlEscaped() + QLatin1String("</pre><pre>") + spaces + QLatin1String("</pre></qt>");
    QMessageBox::information(0, Theme::instance()->appNameGUI(), text);
}

#else

static void displayHelpText(const QString &t)
{
    std::cout << qUtf8Printable(t);
}
#endif

void Application::showHelp()
{
    setHelp();
    QString helpText;
    QTextStream stream(&helpText);
    stream << _theme->appName()
           << QLatin1String(" version ")
           << _theme->version() << endl;

    stream << QLatin1String("File synchronisation desktop utility.") << endl
           << endl
           << QLatin1String(optionsC);

    if (_theme->appName() == QLatin1String("ownCloud"))
        stream << endl
               << "For more information, see http://www.owncloud.org" << endl
               << endl;

    displayHelpText(helpText);
}

void Application::showVersion()
{
    displayHelpText(Theme::instance()->versionSwitchOutput());
}

void Application::showHint(std::string errorHint)
{
    static QString binName = QFileInfo(QCoreApplication::applicationFilePath()).fileName();
    std::cerr << errorHint << std::endl;
    std::cerr << "Try '" << binName.toStdString() << " --help' for more information" << std::endl;
    std::exit(1);
}

bool Application::debugMode()
{
    return _debugMode;
}

void Application::setHelp()
{
    _helpOnly = true;
}

bool Application::giveHelp()
{
    return _helpOnly;
}

bool Application::versionOnly()
{
    return _versionOnly;
}

void Application::showSettingsDialog()
{
    _gui->slotShowParametersDialog();
}

void Application::setAlert(bool alert)
{
    _alert = alert;
    if (_gui)
        _gui->slotComputeOverallSyncStatus();
}

bool Application::getAlert()
{
    return _alert;
}

void Application::updateSystrayIcon()
{
    if (_theme && _theme->systrayUseMonoIcons()) {
        slotUseMonoIconsChanged(_theme->systrayUseMonoIcons());
    }
}

void Application::openVirtualFile(const QString &filename)
{
    QString virtualFileExt = QStringLiteral(APPLICATION_DOTVIRTUALFILE_SUFFIX);
    if (!filename.endsWith(virtualFileExt)) {
        qWarning(lcApplication) << "Can only handle file ending in .owncloud. Unable to open" << filename;
        return;
    }
    QString relativePath;
    auto folder = FolderMan::instance()->folderForPath(filename, &relativePath);
    if (!folder) {
        qWarning(lcApplication) << "Can't find sync folder for" << filename;
        // TODO: show a QMessageBox for errors
        return;
    }
    folder->implicitlyHydrateFile(relativePath);
    QString normalName = filename.left(filename.size() - virtualFileExt.size());
    auto con = QSharedPointer<QMetaObject::Connection>::create();
    *con = QObject::connect(folder, &Folder::syncFinished, [=] {
        QObject::disconnect(*con);
        if (QFile::exists(normalName)) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(normalName));
        }
    });
}

void Application::tryTrayAgain()
{
    qCInfo(lcApplication) << "Trying tray icon, tray available:" << QSystemTrayIcon::isSystemTrayAvailable();
    _gui->hideAndShowTray();
}

bool Application::event(QEvent *event)
{
#ifdef Q_OS_MAC
    if (event->type() == QEvent::FileOpen) {
        QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
        qCDebug(lcApplication) << "QFileOpenEvent" << openEvent->file();
        // virtual file, open it after the Folder were created (if the app is not terminated)
        QString fn = openEvent->file().normalized(QString::NormalizationForm_C);
        QTimer::singleShot(0, this, [this, fn] { openVirtualFile(fn); });
    }
#endif
    return SharedTools::QtSingleApplication::event(event);
}

} // namespace OCC
