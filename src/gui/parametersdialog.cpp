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

#undef CONSOLE_DEBUG
#ifdef CONSOLE_DEBUG
#include <iostream>
#endif

#include "parametersdialog.h"
#include "erroritemwidget.h"
#include "actionwidget.h"
#include "accountmanager.h"
#include "folderman.h"
#include "openfilemanager.h"
#include "progressdispatcher.h"
#include "debugreporter.h"
#include "guiutility.h"
#include "theme.h"
#include "logger.h"

#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QUrl>
#include <QVBoxLayout>

namespace KDC {

static const int boxHMargin= 20;
static const int boxVTMargin = 15;
static const int boxVBMargin = 5;
static const int boxVSpacing = 20;
static const int maxSynchronizedItems = 1000;
static const int maxLogFilesToSend = 25;

Q_LOGGING_CATEGORY(lcParametersDialog, "parametersdialog", QtInfoMsg)

ParametersDialog::ParametersDialog(QWidget *parent)
    : CustomDialog(false, parent)
    , _currentAccountId(QString())
    , _backgroundMainColor(QColor())
    , _pageStackedWidget(nullptr)
    , _mainMenuBarWidget(nullptr)
    , _driveMenuBarWidget(nullptr)
    , _errorsMenuBarWidget(nullptr)
    , _mainStackedWidget(nullptr)
    , _drivesWidget(nullptr)
    , _preferencesWidget(nullptr)
    , _drivePreferencesWidget(nullptr)
    , _errorsStackedWidget(nullptr)
{
    initUI();

    connect(this, &ParametersDialog::exit, this, &ParametersDialog::onExit);
    connect(OCC::FolderMan::instance(), &OCC::FolderMan::folderSyncStateChange,
            this, &ParametersDialog::onRefreshAccountList);
    connect(OCC::AccountManager::instance(), &OCC::AccountManager::accountAdded,
            this, &ParametersDialog::onRefreshAccountList);
    connect(OCC::AccountManager::instance(), &OCC::AccountManager::accountRemoved,
            this, &ParametersDialog::onRefreshAccountList);
    connect(OCC::ProgressDispatcher::instance(), &OCC::ProgressDispatcher::progressInfo,
            this, &ParametersDialog::onUpdateProgress);
    connect(OCC::ProgressDispatcher::instance(), &OCC::ProgressDispatcher::itemCompleted,
            this, &ParametersDialog::onItemCompleted);
}

void ParametersDialog::openErrorPage(const QString &accountId)
{
    onDisplayDriveErrors(accountId);
}

void ParametersDialog::initUI()
{
    /*
     *  _pageStackedWidget
     *      mainPageWidget
     *          mainVBox
     *              _mainMenuBarWidget
     *              _mainStackedWidget
     *                  _drivesWidget
     *                  preferencesScrollArea
     *                      _preferencesWidget
     *      drivePageWidget
     *          driveVBox
     *              _driveMenuBarWidget
     *              drivePreferencesScrollArea
     *                  _drivePreferencesWidget
     *      errorsPageWidget
     *          errorsVBox
     *              _errorsMenuBarWidget
     *              errorsHeaderVBox
     *                  sendLogsWidget
     *                  historyLabel
     *              _errorsStackedWidget
     *                  errorsListWidget[]
     */

    // Page stacked widget
    _pageStackedWidget = new QStackedWidget(this);
    mainLayout()->addWidget(_pageStackedWidget);

    //
    // Main widget
    //
    QWidget *mainPageWidget = new QWidget(this);
    mainPageWidget->setContentsMargins(0, 0, 0, 0);
    _pageStackedWidget->insertWidget(Page::Main, mainPageWidget);

    QVBoxLayout *mainVBox = new QVBoxLayout();
    mainVBox->setContentsMargins(0, 0, 0, 0);
    mainVBox->setSpacing(0);
    mainPageWidget->setLayout(mainVBox);

    // Main menu bar
    _mainMenuBarWidget = new MainMenuBarWidget(this);
    mainVBox->addWidget(_mainMenuBarWidget);

    // Main stacked widget
    _mainStackedWidget = new QStackedWidget(this);
    _mainStackedWidget->setObjectName("mainStackedWidget");
    mainVBox->addWidget(_mainStackedWidget);
    mainVBox->setStretchFactor(_mainStackedWidget, 1);

    // Drives list
    _drivesWidget = new DrivesWidget(this);
    _mainStackedWidget->insertWidget(MainStackedWidget::Drives, _drivesWidget);

    // Preferences
    _preferencesWidget = new PreferencesWidget(this);

    QScrollArea *preferencesScrollArea = new QScrollArea(this);
    preferencesScrollArea->setWidget(_preferencesWidget);
    preferencesScrollArea->setWidgetResizable(true);

    _mainStackedWidget->insertWidget(MainStackedWidget::Preferences, preferencesScrollArea);

    //
    // Drive preferences page widget
    //
    QWidget *drivePageWidget = new QWidget(this);
    drivePageWidget->setContentsMargins(0, 0, 0, 0);
    _pageStackedWidget->insertWidget(Page::Drive, drivePageWidget);

    QVBoxLayout *driveVBox = new QVBoxLayout();
    driveVBox->setContentsMargins(0, 0, 0, 0);
    driveVBox->setSpacing(0);
    drivePageWidget->setLayout(driveVBox);

    // Drive menu bar
    _driveMenuBarWidget = new DriveMenuBarWidget(this);
    driveVBox->addWidget(_driveMenuBarWidget);

    // Drive preferences
    _drivePreferencesWidget = new DrivePreferencesWidget(this);

    QScrollArea *drivePreferencesScrollArea = new QScrollArea(this);
    drivePreferencesScrollArea->setWidget(_drivePreferencesWidget);
    drivePreferencesScrollArea->setWidgetResizable(true);

    driveVBox->addWidget(drivePreferencesScrollArea);
    driveVBox->setStretchFactor(drivePreferencesScrollArea, 1);

    //
    // Errors page widget
    //
    QWidget *errorsPageWidget = new QWidget(this);
    errorsPageWidget->setContentsMargins(0, 0, 0, 0);
    _pageStackedWidget->insertWidget(Page::Errors, errorsPageWidget);

    QVBoxLayout *errorsVBox = new QVBoxLayout();
    errorsVBox->setContentsMargins(0, 0, 0, 0);
    errorsVBox->setSpacing(0);
    errorsPageWidget->setLayout(errorsVBox);

    // Error menu bar
    _errorsMenuBarWidget = new ErrorsMenuBarWidget(this);
    errorsVBox->addWidget(_errorsMenuBarWidget);

    // Errors header
    QWidget *errorsHeaderWidget = new QWidget(this);
    errorsHeaderWidget->setContentsMargins(0, 0, 0, 0);
    errorsHeaderWidget->setObjectName("errorsHeaderWidget");
    errorsVBox->addWidget(errorsHeaderWidget);

    QVBoxLayout *errorsHeaderVBox = new QVBoxLayout();
    errorsHeaderVBox->setContentsMargins(boxHMargin, boxVTMargin, boxHMargin, boxVBMargin);
    errorsHeaderVBox->setSpacing(boxVSpacing);
    errorsHeaderWidget->setLayout(errorsHeaderVBox);

    ActionWidget *sendLogsWidget = new ActionWidget(":/client/resources/icons/actions/help.svg",
                                                    tr("Need help? Generate an archive of the application logs to send it to our support"), this);
    sendLogsWidget->setObjectName("sendLogsWidget");
    errorsHeaderVBox->addWidget(sendLogsWidget);

    QLabel *historyLabel = new QLabel(tr("History"), this);
    historyLabel->setObjectName("blocLabel");
    errorsHeaderVBox->addWidget(historyLabel);

    // Errors stacked widget
    _errorsStackedWidget = new QStackedWidget(this);
    _errorsStackedWidget->setObjectName("errorsStackedWidget");
    errorsVBox->addWidget(_errorsStackedWidget);
    errorsVBox->setStretchFactor(_errorsStackedWidget, 1);

    connect(_mainMenuBarWidget, &MainMenuBarWidget::drivesButtonClicked, this, &ParametersDialog::onDrivesButtonClicked);
    connect(_mainMenuBarWidget, &MainMenuBarWidget::preferencesButtonClicked, this, &ParametersDialog::onPreferencesButtonClicked);
    connect(_mainMenuBarWidget, &MainMenuBarWidget::openHelp, this, &ParametersDialog::onOpenHelp);
    connect(_drivesWidget, &DrivesWidget::addDrive, this, &ParametersDialog::onAddDrive);
    connect(_drivesWidget, &DrivesWidget::runSync, this, &ParametersDialog::onRunSync);
    connect(_drivesWidget, &DrivesWidget::pauseSync, this, &ParametersDialog::onPauseSync);
    connect(_drivesWidget, &DrivesWidget::resumeSync, this, &ParametersDialog::onResumeSync);
    connect(_drivesWidget, &DrivesWidget::manageOffer, this, &ParametersDialog::onManageOffer);
    connect(_drivesWidget, &DrivesWidget::remove, this, &ParametersDialog::onRemove);
    connect(_drivesWidget, &DrivesWidget::displayDriveParameters, this, &ParametersDialog::onDisplayDriveParameters);
    connect(_drivesWidget, &DrivesWidget::displayDriveErrors, this, &ParametersDialog::onDisplayDriveErrors);
    connect(_preferencesWidget, &PreferencesWidget::setStyle, this, &ParametersDialog::onSetStyle);
    connect(_driveMenuBarWidget, &DriveMenuBarWidget::backButtonClicked, this, &ParametersDialog::onDisplayDrivesList);
    connect(_driveMenuBarWidget, &DriveMenuBarWidget::runSync, this, &ParametersDialog::onRunSync);
    connect(_driveMenuBarWidget, &DriveMenuBarWidget::pauseSync, this, &ParametersDialog::onPauseSync);
    connect(_driveMenuBarWidget, &DriveMenuBarWidget::resumeSync, this, &ParametersDialog::onResumeSync);
    connect(_driveMenuBarWidget, &DriveMenuBarWidget::manageOffer, this, &ParametersDialog::onManageOffer);
    connect(_driveMenuBarWidget, &DriveMenuBarWidget::remove, this, &ParametersDialog::onRemove);
    connect(_drivePreferencesWidget, &DrivePreferencesWidget::displayErrors, this, &ParametersDialog::onDisplayDriveErrors);
    connect(_errorsMenuBarWidget, &ErrorsMenuBarWidget::backButtonClicked, this, &ParametersDialog::onDisplayDrivesList);
    connect(sendLogsWidget, &ActionWidget::clicked, this, &ParametersDialog::onSendLogs);
}

QByteArray ParametersDialog::contents(const QString &path)
{
    QFile file(path);
    if (file.open(QFile::ReadOnly)) {
        return file.readAll();
    }
    else {
        return QByteArray();
    }
}

void ParametersDialog::onExit()
{
    accept();
}

void ParametersDialog::onRefreshAccountList()
{
#ifdef CONSOLE_DEBUG
    std::cout << QTime::currentTime().toString("hh:mm:ss").toStdString()
              << " - ParametersDialog::onRefreshAccountList" << std::endl;
#endif

    if (OCC::AccountManager::instance()->accounts().isEmpty()) {
        _accountInfoMap.clear();
        _drivesWidget->clear();
    }
    else {
        for (OCC::AccountStatePtr accountStatePtr : OCC::AccountManager::instance()->accounts()) {
            if (accountStatePtr && !accountStatePtr->account().isNull()) {
                QString accountId = accountStatePtr->account()->id();
                auto accountInfoIt = _accountInfoMap.find(accountId);
                if (accountInfoIt == _accountInfoMap.end()) {
                    // New account
                    AccountInfoParameters accountInfo(accountStatePtr.data());
                    connect(accountInfo._quotaInfoPtr.get(), &OCC::QuotaInfo::quotaUpdated,
                            this, &ParametersDialog::onUpdateQuota);

                    _accountInfoMap[accountId] = accountInfo;
                    accountInfoIt = _accountInfoMap.find(accountId);
                }

                // Set or update account name & color
                accountInfoIt->second._name = accountStatePtr->account()->driveName();
                accountInfoIt->second._color = accountStatePtr->account()->getDriveColor();
                accountInfoIt->second._isSignedIn = !accountStatePtr->isSignedOut();

                OCC::Folder::Map folderMap = OCC::FolderMan::instance()->map();
                for (auto folderIt = folderMap.begin(); folderIt != folderMap.end(); folderIt++) {
                    if (folderIt.value() && folderIt.value()->accountState()) {
                        OCC::AccountPtr folderAccountPtr = folderIt.value()->accountState()->account();
                        if (!folderAccountPtr.isNull()) {
                            if (folderAccountPtr->id() == accountId) {
                                auto folderInfoIt = accountInfoIt->second._folderMap.find(folderIt.key());
                                if (folderInfoIt == accountInfoIt->second._folderMap.end()) {
                                    // New folder
                                    accountInfoIt->second._folderMap[folderIt.key()] = std::unique_ptr<FolderInfo>(
                                            new FolderInfo(folderIt.value()->shortGuiLocalPath(), folderIt.value()->path()));
                                    folderInfoIt = accountInfoIt->second._folderMap.find(folderIt.key());
                                }

                                folderInfoIt->second->_paused = folderIt.value()->syncPaused();
                                folderInfoIt->second->_unresolvedConflicts = folderIt.value()->syncResult().hasUnresolvedConflicts();
                                folderInfoIt->second->_status = folderIt.value()->syncResult().status();
                            }
                        }
                        else {
                            qCDebug(lcParametersDialog) << "Null pointer!";
                            Q_ASSERT(false);
                        }
                    }
                    else {
                        qCDebug(lcParametersDialog) << "Null pointer!";
                        Q_ASSERT(false);
                    }
                }

                // Manage removed folders
                auto folderInfoIt = accountInfoIt->second._folderMap.begin();
                while (folderInfoIt != accountInfoIt->second._folderMap.end()) {
                    if (folderMap.find(folderInfoIt->first) == folderMap.end()) {
                        folderInfoIt = accountInfoIt->second._folderMap.erase(folderInfoIt);
                    }
                    else {
                        folderInfoIt++;
                    }
                }

                // Compute account status
                accountInfoIt->second.updateStatus();

                _drivesWidget->addOrUpdateDrive(accountInfoIt->first, accountInfoIt->second);
            }
            else {
                qCDebug(lcParametersDialog) << "Null pointer!";
                Q_ASSERT(false);
            }
        }

        // Manage removed accounts
        auto accountStatusIt = _accountInfoMap.begin();
        while (accountStatusIt != _accountInfoMap.end()) {
            if (!OCC::AccountManager::instance()->getAccountFromId(accountStatusIt->first)) {
                _drivesWidget->removeDrive(accountStatusIt->first);
                if (accountStatusIt->first == _currentAccountId) {
                    // The current account is removed
                    _currentAccountId = QString();
                    _driveMenuBarWidget->reset();
                    _drivePreferencesWidget->reset();
                    if (_pageStackedWidget->currentIndex() == Page::Drive) {
                        _pageStackedWidget->setCurrentIndex(Page::Main);
                    }
                }
                accountStatusIt = _accountInfoMap.erase(accountStatusIt);
            }
            else {
                accountStatusIt++;
            }
        }
    }
}

void ParametersDialog::onUpdateProgress(const QString &folderId, const OCC::ProgressInfo &progress)
{
    OCC::Folder *folder = OCC::FolderMan::instance()->folder(folderId);
    if (folder) {
#ifdef CONSOLE_DEBUG
        std::cout << QTime::currentTime().toString("hh:mm:ss").toStdString()
                  << " - ParametersDialog::onUpdateProgress folder: " << folder->path().toStdString() << std::endl;
#endif

        if (folder->accountState()) {
            OCC::AccountPtr account = folder->accountState()->account();
            if (!account.isNull()) {
                const auto accountInfoIt = _accountInfoMap.find(account->id());
                if (accountInfoIt != _accountInfoMap.end()) {
                    const auto folderInfoIt = accountInfoIt->second._folderMap.find(folderId);
                    if (folderInfoIt != accountInfoIt->second._folderMap.end()) {
                       if (folderInfoIt->second.get()) {
                            folderInfoIt->second.get()->_currentFile = progress.currentFile();
                            folderInfoIt->second.get()->_totalFiles = qMax(progress.currentFile(), progress.totalFiles());
                            folderInfoIt->second.get()->_completedSize = progress.completedSize();
                            folderInfoIt->second.get()->_totalSize = qMax(progress.completedSize(), progress.totalSize());
                            folderInfoIt->second.get()->_estimatedRemainingTime = progress.totalProgress().estimatedEta;
                            folderInfoIt->second.get()->_paused = folder->syncPaused();
                            folderInfoIt->second.get()->_unresolvedConflicts = folder->syncResult().hasUnresolvedConflicts();
                            folderInfoIt->second.get()->_status = folder->syncResult().status();
                        }
                        else {
                            qCDebug(lcParametersDialog) << "Null pointer!";
                            Q_ASSERT(false);
                        }
                    }

                    // Compute account status
                    accountInfoIt->second.updateStatus();

                    _drivesWidget->addOrUpdateDrive(account->id(), accountInfoIt->second);
                }
            }
            else {
                qCDebug(lcParametersDialog) << "Null pointer!";
                Q_ASSERT(false);
            }
        }
        else {
            qCDebug(lcParametersDialog) << "Null pointer!";
            Q_ASSERT(false);
        }
    }
}

void ParametersDialog::onUpdateQuota(qint64 total, qint64 used)
{
    QString accountId = qvariant_cast<QString>(sender()->property(accountIdProperty));

#ifdef CONSOLE_DEBUG
    std::cout << QTime::currentTime().toString("hh:mm:ss").toStdString()
              << " - ParametersDialog::onUpdateQuota account: " << accountId.toStdString() << std::endl;
#endif

    const auto accountInfoIt = _accountInfoMap.find(accountId);
    if (accountInfoIt != _accountInfoMap.end()) {
        accountInfoIt->second._totalSize = total;
        accountInfoIt->second._used = used;

        if (accountId == _currentAccountId) {
            _drivePreferencesWidget->setUsedSize(total, used);
        }
    }
}

void ParametersDialog::onItemCompleted(const QString &folderId, const OCC::SyncFileItemPtr &item)
{
#ifdef CONSOLE_DEBUG
    std::cout << QTime::currentTime().toString("hh:mm:ss").toStdString()
              << " - ParametersDialog::onItemCompleted" << std::endl;
#endif

    if (!item.isNull()) {
        if (!(item.data()->_status == OCC::SyncFileItem::NoStatus
                || item.data()->_status == OCC::SyncFileItem::FatalError
                || item.data()->_status == OCC::SyncFileItem::NormalError
                || item.data()->_status == OCC::SyncFileItem::SoftError
                || item.data()->_status == OCC::SyncFileItem::DetailError
                || item.data()->_status == OCC::SyncFileItem::BlacklistedError
                || item.data()->_status == OCC::SyncFileItem::FileIgnored)) {
            return;
        }

        OCC::Folder *folder = OCC::FolderMan::instance()->folder(folderId);
        if (folder) {
            if (folder->accountState()) {
                OCC::AccountPtr account = folder->accountState()->account();
                if (!account.isNull()) {
                    const auto accountInfoIt = _accountInfoMap.find(account->id());
                    if (accountInfoIt != _accountInfoMap.end()) {
                        if (!accountInfoIt->second._errorsListWidget) {
                            accountInfoIt->second._errorsListWidget = new QListWidget(this);
                            accountInfoIt->second._errorsListWidget->setSpacing(0);
                            accountInfoIt->second._errorsListWidget->setSelectionMode(QAbstractItemView::NoSelection);
                            accountInfoIt->second._errorsListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
                            accountInfoIt->second._errorsListStackPosition =
                                    _errorsStackedWidget->addWidget(accountInfoIt->second._errorsListWidget);
                        }

                        // Add item to synchronized list
                        QListWidgetItem *widgetItem = new QListWidgetItem();
                        SynchronizedItem synchronizedItem(folderId,
                                                          item.data()->_file,
                                                          item.data()->_fileId,
                                                          item.data()->_status,
                                                          item.data()->_direction,
                                                          accountInfoIt->second.folderPath(folderId, item.data()->_file),
                                                          QDateTime::currentDateTime(),
                                                          item.data()->_errorString);
                        accountInfoIt->second._errorsListWidget->insertItem(0, widgetItem);
                        ErrorItemWidget *widget = new ErrorItemWidget(synchronizedItem,
                                                                      accountInfoIt->second,
                                                                      accountInfoIt->second._errorsListWidget);
                        accountInfoIt->second._errorsListWidget->setItemWidget(widgetItem, widget);
                        // Adjust widgetItem sizeHint because widget has a variable height
                        widgetItem->setSizeHint(widget->size());
                        connect(widget, &ErrorItemWidget::openFolder, this, &ParametersDialog::onOpenFolderItem);

                        if (accountInfoIt->second._errorsListWidget->count() > maxSynchronizedItems) {
                            // Remove last row
                            QListWidgetItem *lastWidgetItem = accountInfoIt->second._errorsListWidget->takeItem(
                                        accountInfoIt->second._errorsListWidget->count() - 1);
                            delete lastWidgetItem;
                        }

                        if (_currentAccountId == accountInfoIt->first
                                && _pageStackedWidget->currentIndex() == Page::Drive) {
                            emit _drivePreferencesWidget->errorAdded();
                        }
                    }
                }
                else {
                    qCDebug(lcParametersDialog) << "Null pointer!";
                    Q_ASSERT(false);
                }
            }
            else {
                qCDebug(lcParametersDialog) << "Null pointer!";
                Q_ASSERT(false);
            }
        }
    }
    else {
        qCDebug(lcParametersDialog) << "Null pointer!";
        Q_ASSERT(false);
    }
}

void ParametersDialog::onDrivesButtonClicked()
{
    _mainStackedWidget->setCurrentIndex(MainStackedWidget::Drives);
}

void ParametersDialog::onPreferencesButtonClicked()
{
    _mainStackedWidget->setCurrentIndex(MainStackedWidget::Preferences);
}

void ParametersDialog::onOpenHelp()
{
    QDesktopServices::openUrl(QUrl(OCC::Theme::instance()->helpUrl()));
}

void ParametersDialog::onAddDrive()
{
    emit addDrive();
}

void ParametersDialog::onRunSync(const QString &accountId)
{
    OCC::Utility::runSync(accountId, QString());
}

void ParametersDialog::onPauseSync(const QString &accountId)
{
    OCC::Utility::pauseSync(accountId, QString(), true);
}

void ParametersDialog::onResumeSync(const QString &accountId)
{
    OCC::Utility::pauseSync(accountId, QString(), false);
}

void ParametersDialog::onManageOffer(const QString &accountId)
{

}

void ParametersDialog::onRemove(const QString &accountId)
{
    OCC::AccountManager *accountManager = OCC::AccountManager::instance();
    OCC::AccountStatePtr accountStatePtr = accountManager->getAccountStateFromId(accountId);
    if (accountStatePtr.data()) {
        QMessageBox msgBox(
                    QMessageBox::Question, tr("Confirm Account Removal"),
                    tr("<p>Do you really want to remove the connection to the account <i>%1</i>?</p>"
                       "<p><b>Note:</b> This will <b>not</b> delete any files.</p>")
                        .arg(accountStatePtr->account()->driveName()),
                    QMessageBox::NoButton, this);
        msgBox.setWindowModality(Qt::WindowModal);
        QPushButton *yesButton = msgBox.addButton(tr("Remove connection"), QMessageBox::YesRole);
        msgBox.addButton(tr("Cancel"), QMessageBox::NoRole);
        msgBox.exec();
        if (msgBox.clickedButton() != yesButton) {
            return;
        }

        accountManager->deleteAccount(accountStatePtr.data());
        accountManager->save();
    }
}

void ParametersDialog::onDisplayDriveParameters(const QString &accountId)
{
    auto accountInfoIt = _accountInfoMap.find(accountId);
    if (accountInfoIt != _accountInfoMap.end()) {
        _currentAccountId = accountInfoIt->first;
        _driveMenuBarWidget->setAccount(accountInfoIt->first, &accountInfoIt->second);
        _drivePreferencesWidget->setAccount(accountInfoIt->first, &accountInfoIt->second,
                                            accountInfoIt->second._errorsListWidget != nullptr);
        _pageStackedWidget->setCurrentIndex(Page::Drive);
    }
}

void ParametersDialog::onDisplayDriveErrors(const QString &accountId)
{
    auto accountInfoIt = _accountInfoMap.find(accountId);
    if (accountInfoIt != _accountInfoMap.end()) {
        _errorsMenuBarWidget->setAccount(accountInfoIt->first, &accountInfoIt->second);
        _errorsStackedWidget->setCurrentIndex(accountInfoIt->second._errorsListStackPosition);
        _pageStackedWidget->setCurrentIndex(Page::Errors);
    }
}

void ParametersDialog::onSetStyle(bool darkTheme)
{
    emit setStyle(darkTheme);
}

void ParametersDialog::onDisplayDrivesList()
{
    _pageStackedWidget->setCurrentIndex(Page::Main);
}

void ParametersDialog::onSendLogs()
{
    if (OCC::Theme::instance()->debugReporterUrl().isEmpty()) {
        Q_ASSERT(false);
        return;
    }

    QMessageBox msgBox(QMessageBox::Information, QString(),
                       tr("Please confirm the transmission of debugging information to our support."),
                       QMessageBox::Yes | QMessageBox::No, this);
    msgBox.setWindowModality(Qt::WindowModal);
    msgBox.setDefaultButton(QMessageBox::Yes);
    if (msgBox.exec() == QMessageBox::No) {
        return;
    }

    OCC::DebugReporter *debugReporter = new OCC::DebugReporter(QUrl(OCC::Theme::instance()->debugReporterUrl()), this);

    // Write accounts
    auto accountList = OCC::AccountManager::instance()->accounts();
    int num = 0;
    foreach (OCC::AccountStatePtr account, accountList) {
        num++;
        debugReporter->setReportData(OCC::DebugReporter::MapKeyType::DriveId, num, account->account()->driveId().toUtf8());
        debugReporter->setReportData(OCC::DebugReporter::MapKeyType::DriveName, num, account->account()->driveName().toUtf8());
        debugReporter->setReportData(OCC::DebugReporter::MapKeyType::UserId, num, account->account()->davUser().toUtf8());
        debugReporter->setReportData(OCC::DebugReporter::MapKeyType::UserName, num, account->account()->davDisplayName().toUtf8());
    }

    if (num == 0) {
        qCDebug(lcParametersDialog()) << "No account";
        return;
    }

    // Write logs
    QString temporaryFolderLogDirPath = OCC::Logger::instance()->temporaryFolderLogDirPath();
    QDir dir(temporaryFolderLogDirPath);
    if (dir.exists()) {
        QStringList files = dir.entryList(QStringList("*owncloud.log.*.gz"), QDir::Files, QDir::Name | QDir::Reversed);
        num = 0;
        for (const QString &file : files) {
            num++;
            if (num > maxLogFilesToSend) {
                break;
            }
            debugReporter->setReportData(OCC::DebugReporter::MapKeyType::LogName, num,
                contents(temporaryFolderLogDirPath + "/" + file),
                "application/octet-stream",
                QFileInfo(file).fileName().toUtf8());
        }

        if (num == 0) {
            qCDebug(lcParametersDialog()) << "No log file";
            return;
        }
    }
    else {
        qCDebug(lcParametersDialog()) << "Empty log dir: " << temporaryFolderLogDirPath;
        return;
    }

    connect(debugReporter, &OCC::DebugReporter::sent, this, &ParametersDialog::onDebugReporterDone);
    debugReporter->send();
}

void ParametersDialog::onOpenFolderItem(const QString &filePath)
{
    if (!filePath.isEmpty()) {
        QFileInfo fileInfo(filePath);
        if (fileInfo.exists()) {
            OCC::showInFileManager(fileInfo.filePath());
        }
        else if (fileInfo.dir().exists()) {
            QUrl url = OCC::Utility::getUrlFromLocalPath(fileInfo.dir().path());
            if (url.isValid()) {
                if (!QDesktopServices::openUrl(url)) {
                    qCWarning(lcParametersDialog) << "QDesktopServices::openUrl failed for " << url.toString();
                    QMessageBox msgBox(QMessageBox::Warning, QString(),
                                tr("Unable to open folder path %1.").arg(url.toString()),
                                QMessageBox::Ok, this);
                    msgBox.setWindowModality(Qt::WindowModal);
                    msgBox.exec();
                }
            }
        }
    }
}

void ParametersDialog::onDebugReporterDone(bool retCode, const QString &debugId)
{
    QMessageBox msgBox(QMessageBox::Information, QString(),
                       retCode
                        ? tr("Transmission done!<br>Please refer to identifier <b>%1</b> in bug reports.").arg(debugId)
                        : tr("Transmission failed!"),
                       QMessageBox::Ok, this);
    msgBox.setWindowModality(Qt::WindowModal);
    msgBox.setTextInteractionFlags(Qt::TextSelectableByMouse);
    msgBox.exec();
}

ParametersDialog::AccountInfoParameters::AccountInfoParameters()
    : AccountInfo()
    , _errorsListWidget(nullptr)
    , _errorsListStackPosition(0)
{
}

ParametersDialog::AccountInfoParameters::AccountInfoParameters(OCC::AccountState *accountState)
    : AccountInfo(accountState)
    , _errorsListWidget(nullptr)
    , _errorsListStackPosition(0)
{
}

}
