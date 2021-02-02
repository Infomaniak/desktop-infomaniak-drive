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

#include "drivepreferenceswidget.h"
#include "localfolderdialog.h"
#include "serverbasefolderdialog.h"
#include "serverfoldersdialog.h"
#include "confirmsynchronizationdialog.h"
#include "bigfoldersdialog.h"
#include "custommessagebox.h"
#include "custompushbutton.h"
#include "accountmanager.h"
#include "configfile.h"
#include "guiutility.h"
#include "theme.h"
#include "common/utility.h"
#include "common/vfs.h"
#include "filesystem.h"

#include <QDesktopServices>
#include <QDir>
#include <QIcon>
#include <QLabel>
#include <QStandardPaths>
#include <QStyle>

namespace KDC {

static const int boxHMargin= 20;
static const int boxVMargin = 20;
static const int boxVSpacing = 12;
static const int textHSpacing = 5;
static const int avatarSize = 40;

static const QString folderBlocName("folderBloc");

Q_LOGGING_CATEGORY(lcDrivePreferencesWidget, "gui.drivepreferenceswidget", QtInfoMsg)

DrivePreferencesWidget::DrivePreferencesWidget(QWidget *parent)
    : QWidget(parent)
    , _accountId(QString())
    , _accountInfo(nullptr)
    , _mainVBox(nullptr)
    , _displayErrorsWidget(nullptr)
    , _displayBigFoldersWarningWidget(nullptr)
    , _smartSyncSwitch(nullptr)
    , _smartSyncDescriptionLabel(nullptr)
    , _accountAvatarLabel(nullptr)
    , _accountNameLabel(nullptr)
    , _accountMailLabel(nullptr)
    , _notificationsSwitch(nullptr)
    , _foldersBeginIndex(0)
{
    setContentsMargins(0, 0, 0, 0);

    OCC::ConfigFile cfg;

    /*
     *  _mainVBox
     *      _displayErrorsWidget
     *      _displayBigFoldersWarningWidget
     *      foldersHeaderHBox
     *          foldersLabel
     *          addFolderButton
     *      folderBloc[]
     *      synchronizationLabel
     *      synchronizationBloc
     *          smartSyncBox
     *              smartSync1HBox
     *                  smartSyncLabel
     *                  _smartSyncSwitch
     *              _smartSyncDescriptionLabel
     *          driveFoldersWidget
     *              driveFoldersVBox
     *                  driveFoldersLabel
     *                  driveFoldersStatusLabel
     *      locationLabel
     *      locationBloc
     *          _locationWidget
     *              _locationLayout
     *      notificationsLabel
     *      notificationsBloc
     *          notificationsBox
     *              notifications1HBox
     *                  notificationsTitleLabel
     *                  _notificationsSwitch
     *              notifications2HBox
     *                  notificationsDescriptionLabel
     *      connectedWithLabel
     *      connectedWithBloc
     *          connectedWithBox
     *              _accountAvatarLabel
     *              connectedWithVBox
     *                  _accountNameLabel
     *                  _accountMailLabel
     */

    _mainVBox = new QVBoxLayout();
    _mainVBox->setContentsMargins(boxHMargin, boxVMargin, boxHMargin, boxVMargin);
    _mainVBox->setSpacing(boxVSpacing);
    setLayout(_mainVBox);

    //
    // Synchronization errors
    //
    _displayErrorsWidget = new ActionWidget(":/client/resources/icons/actions/warning.svg",
                                            tr("Some files couldn't be synchronized"), this);
    _displayErrorsWidget->setObjectName("displayErrorsWidget");
    _mainVBox->addWidget(_displayErrorsWidget);

    //
    // Big folders warning
    //
    _displayBigFoldersWarningWidget = new ActionWidget(":/client/resources/icons/actions/warning.svg",
                                                       tr("Some folders were not synchronized because they are too large."), this);
    _displayBigFoldersWarningWidget->setObjectName("displayBigFoldersWarningWidget");
    _displayBigFoldersWarningWidget->setVisible(false);
    _mainVBox->addWidget(_displayBigFoldersWarningWidget);

    //
    // Folders blocs
    //
    QHBoxLayout *foldersHeaderHBox = new QHBoxLayout();
    foldersHeaderHBox->setContentsMargins(0, 0, 0, 0);
    _mainVBox->addLayout(foldersHeaderHBox);
    _foldersBeginIndex = _mainVBox->indexOf(foldersHeaderHBox) + 1;

    QLabel *foldersLabel = new QLabel(tr("Folders"), this);
    foldersLabel->setObjectName("blocLabel");
    foldersHeaderHBox->addWidget(foldersLabel);
    foldersHeaderHBox->addStretch();

    CustomPushButton *addFolderButton = new CustomPushButton(":/client/resources/icons/actions/add.svg",
                                                            tr("Add an advanced synchronization"), this);
    addFolderButton->setObjectName("addFolderButton");
    foldersHeaderHBox->addWidget(addFolderButton);

    //
    // Synchronization bloc
    //

    if (OCC::bestAvailableVfsMode(OCC::ConfigFile().showExperimentalOptions()) != OCC::Vfs::Off) {
        QLabel *synchronizationLabel = new QLabel(tr("Synchronization"), this);
        synchronizationLabel->setObjectName("blocLabel");
        _mainVBox->addWidget(synchronizationLabel);

        PreferencesBlocWidget *synchronizationBloc = new PreferencesBlocWidget(this);
        _mainVBox->addWidget(synchronizationBloc);

        QBoxLayout *smartSyncBox = synchronizationBloc->addLayout(QBoxLayout::Direction::TopToBottom);

        QHBoxLayout *smartSync1HBox = new QHBoxLayout();
        smartSync1HBox->setContentsMargins(0, 0, 0, 0);
        smartSync1HBox->setSpacing(0);
        smartSyncBox->addLayout(smartSync1HBox);

        QLabel *smartSyncLabel = new QLabel(tr("Activate Lite Sync"), this);
        smartSync1HBox->addWidget(smartSyncLabel);
        smartSync1HBox->addStretch();

        _smartSyncSwitch = new CustomSwitch(this);
        _smartSyncSwitch->setLayoutDirection(Qt::RightToLeft);
        _smartSyncSwitch->setAttribute(Qt::WA_MacShowFocusRect, false);
        smartSync1HBox->addWidget(_smartSyncSwitch);

        _smartSyncDescriptionLabel = new QLabel(this);
        _smartSyncDescriptionLabel->setObjectName("description");
        _smartSyncDescriptionLabel->setText(tr("Synchronize all your files without using your computer space."
                                              " <a style=\"%1\" href=\"%2\">Learn more</a>")
                                            .arg(OCC::Utility::linkStyle)
                                            .arg(OCC::Utility::learnMoreLink));
        _smartSyncDescriptionLabel->setWordWrap(true);
        smartSyncBox->addWidget(_smartSyncDescriptionLabel);
    }

    //
    // Notifications bloc
    //
    QLabel *notificationsLabel = new QLabel(tr("Notifications"), this);
    notificationsLabel->setObjectName("blocLabel");
    _mainVBox->addWidget(notificationsLabel);

    PreferencesBlocWidget *notificationsBloc = new PreferencesBlocWidget(this);
    _mainVBox->addWidget(notificationsBloc);

    QBoxLayout *notificationsBox = notificationsBloc->addLayout(QBoxLayout::Direction::TopToBottom);

    QHBoxLayout *notifications1HBox = new QHBoxLayout();
    notifications1HBox->setContentsMargins(0, 0, 0, 0);
    notifications1HBox->setSpacing(0);
    notificationsBox->addLayout(notifications1HBox);

    QLabel *notificationsTitleLabel = new QLabel(tr("Enable the notifications for this kDrive"), this);
    notifications1HBox->addWidget(notificationsTitleLabel);

    _notificationsSwitch = new CustomSwitch(this);
    _notificationsSwitch->setLayoutDirection(Qt::RightToLeft);
    _notificationsSwitch->setAttribute(Qt::WA_MacShowFocusRect, false);
    notifications1HBox->addWidget(_notificationsSwitch);

    QHBoxLayout *notifications2HBox = new QHBoxLayout();
    notifications2HBox->setContentsMargins(0, 0, 0, 0);
    notifications2HBox->setSpacing(0);
    notificationsBox->addLayout(notifications2HBox);

    QLabel *notificationsDescriptionLabel = new QLabel(tr("A notification will be displayed as soon as a new folder "
                                                          "has been synchronized or modified"), this);
    notificationsDescriptionLabel->setObjectName("description");
    notificationsDescriptionLabel->setWordWrap(true);
    notifications2HBox->addWidget(notificationsDescriptionLabel);

    //
    // Connected with bloc
    //
    QLabel *connectedWithLabel = new QLabel(tr("Connected with"), this);
    connectedWithLabel->setObjectName("blocLabel");
    _mainVBox->addWidget(connectedWithLabel);

    PreferencesBlocWidget *connectedWithBloc = new PreferencesBlocWidget(this);
    _mainVBox->addWidget(connectedWithBloc);

    QBoxLayout *connectedWithBox = connectedWithBloc->addLayout(QBoxLayout::Direction::LeftToRight);

    _accountAvatarLabel = new QLabel(this);
    _accountAvatarLabel->setObjectName("accountAvatarLabel");
    connectedWithBox->addWidget(_accountAvatarLabel);

    QVBoxLayout *connectedWithVBox = new QVBoxLayout();
    connectedWithVBox->setContentsMargins(0, 0, 0, 0);
    connectedWithVBox->setSpacing(textHSpacing);
    connectedWithBox->addLayout(connectedWithVBox);

    _accountNameLabel = new QLabel(this);
    _accountNameLabel->setObjectName("accountNameLabel");
    connectedWithVBox->addWidget(_accountNameLabel);

    _accountMailLabel = new QLabel(this);
    _accountMailLabel->setObjectName("accountMailLabel");
    connectedWithVBox->addWidget(_accountMailLabel);

    connectedWithBox->addStretch();

    CustomToolButton *removeDriveButton = new CustomToolButton(this);
    removeDriveButton->setIconPath(":/client/resources/icons/actions/error-sync.svg");
    removeDriveButton->setToolTip(tr("Remove synchronization"));
    connectedWithBox->addWidget(removeDriveButton);

    _mainVBox->addStretch();

    connect(_displayErrorsWidget, &ActionWidget::clicked, this, &DrivePreferencesWidget::onErrorsWidgetClicked);
    connect(_displayBigFoldersWarningWidget, &ActionWidget::clicked, this, &DrivePreferencesWidget::onBigFoldersWarningWidgetClicked);
    connect(addFolderButton, &CustomPushButton::clicked, this, &DrivePreferencesWidget::onAddFolder);
    if (_smartSyncSwitch && _smartSyncDescriptionLabel) {
        connect(_smartSyncSwitch, &CustomSwitch::clicked, this, &DrivePreferencesWidget::onSmartSyncSwitchClicked);
        connect(_smartSyncDescriptionLabel, &QLabel::linkActivated, this, &DrivePreferencesWidget::onLinkActivated);
    }
    connect(_notificationsSwitch, &CustomSwitch::clicked, this, &DrivePreferencesWidget::onNotificationsSwitchClicked);
    connect(this, &DrivePreferencesWidget::errorAdded, this, &DrivePreferencesWidget::onErrorAdded);
    connect(removeDriveButton, &CustomToolButton::clicked, this, &DrivePreferencesWidget::onRemoveDrive);
    connect(this, &DrivePreferencesWidget::newBigFolderDiscovered, this, &DrivePreferencesWidget::onNewBigFolderDiscovered);
    connect(this, &DrivePreferencesWidget::undecidedListsCleared, this, &DrivePreferencesWidget::onUndecidedListsCleared);
}

void DrivePreferencesWidget::setAccount(const QString &accountId, const AccountInfo *accountInfo, bool errors)
{
    if (_accountId != accountId) {
        reset();
    }

    _accountId = accountId;
    _accountInfo = accountInfo;
    _displayErrorsWidget->setVisible(errors);
    _displayBigFoldersWarningWidget->setVisible(existUndecidedList());
    updateFoldersBlocs();
    updateSmartSyncSwitchState();
    _notificationsSwitch->setChecked(!OCC::FolderMan::instance()->notificationsDisabled(_accountId));
    updateAccountInfo();
}

void DrivePreferencesWidget::reset()
{
    _accountId = QString();
    _accountInfo = nullptr;
    _displayErrorsWidget->setVisible(false);
    _displayBigFoldersWarningWidget->setVisible(false);
    resetFoldersBlocs();
    if (_smartSyncSwitch) {
        _smartSyncSwitch->setChecked(false);
    }
    _notificationsSwitch->setChecked(true);
    _accountAvatarLabel->setPixmap(QPixmap());
    _accountNameLabel->setText(QString());
    _accountMailLabel->setText(QString());
}

void DrivePreferencesWidget::updateSmartSyncSwitchState()
{
    if (_smartSyncSwitch) {
        bool smartSync = isSmartSyncActivated();
        bool oneHasVfsOnOffSwitchPending = false;
        for (auto folderInfoElt : _accountInfo->_folderMap) {
            OCC::Folder *folder = OCC::FolderMan::instance()->folder(folderInfoElt.first);
            if (folder) {
                oneHasVfsOnOffSwitchPending |= folder->isVfsOnOffSwitchPending();
            }
        }
        _smartSyncSwitch->setCheckState(smartSync ? Qt::Checked : Qt::Unchecked);
        _smartSyncSwitch->setEnabled(!oneHasVfsOnOffSwitchPending);
        if (!oneHasVfsOnOffSwitchPending) {
            _smartSyncSwitch->setToolTip("");
        }

        QList<PreferencesBlocWidget *> folderBlocList = findChildren<PreferencesBlocWidget *>(folderBlocName);
        for (PreferencesBlocWidget *folderBloc : folderBlocList) {
            FolderItemWidget *itemWidget = blocItemWidget(folderBloc);
            if (itemWidget) {
                itemWidget->setSmartSync(smartSync);
            }
        }
    }    
}

bool DrivePreferencesWidget::existUndecidedList()
{
    bool ret = false;
    if (_accountInfo) {
        for (auto folderInfoElt : _accountInfo->_folderMap) {
            OCC::Folder *folder = OCC::FolderMan::instance()->folder(folderInfoElt.first);
            if (folder) {
                bool ok;
                QStringList undecidedList = folder->journalDb()->getSelectiveSyncList(OCC::SyncJournalDb::SelectiveSyncUndecidedList, &ok);
                if (!ok) {
                    qCWarning(lcDrivePreferencesWidget) << "Could not read selective sync list from db.";
                    break;
                }
                if (undecidedList.size() > 0) {
                    ret = true;
                    break;
                }
            }
        }
    }

    return ret;
}

void DrivePreferencesWidget::updateAccountInfo()
{
    OCC::AccountPtr accountPtr = OCC::AccountManager::instance()->getAccountFromId(_accountId);
    if (!accountPtr.isNull()) {
        if (!accountPtr->avatar().isNull()) {
            _accountAvatarLabel->setPixmap(OCC::Utility::getAvatarFromImage(accountPtr->avatar())
                                           .scaled(avatarSize, avatarSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }

        _accountNameLabel->setText(accountPtr->davDisplayName());

        QString email = OCC::AccountManager::instance()->getUserId(accountPtr);
        _accountMailLabel->setText(email);
    }
    else {
        qCDebug(lcDrivePreferencesWidget) << "Null pointer!";
    }
}

void DrivePreferencesWidget::askEnableSmartSync(const std::function<void (bool)> &callback)
{
    const auto bestVfsMode = OCC::bestAvailableVfsMode(OCC::ConfigFile().showExperimentalOptions());
    CustomMessageBox *msgBox = nullptr;
    if (bestVfsMode == OCC::Vfs::WindowsCfApi || bestVfsMode == OCC::Vfs::WithSuffix) {
        msgBox = new CustomMessageBox(
                    QMessageBox::Question,
                    tr("Do you really want to turn on Lite Sync?"),
                     QMessageBox::NoButton, this);
        msgBox->addButton(tr("CONFIRM"), QMessageBox::Yes);
        msgBox->addButton(tr("CANCEL"), QMessageBox::No);
        msgBox->setDefaultButton(QMessageBox::Yes);
    }
    int result = msgBox->exec();
    callback(result == QMessageBox::Yes);
}

void DrivePreferencesWidget::askDisableSmartSync(const std::function<void (bool, bool)> &callback)
{
    // Available space
    qint64 freeSize = OCC::Utility::freeDiskSpace(dirSeparator);

    // Compute folders size sum
    qint64 localFoldersSize = 0;
    qint64 localFoldersDiskSize = 0;
    for (auto folderInfoElt : _accountInfo->_folderMap) {
        OCC::Folder *folder = OCC::FolderMan::instance()->folder(folderInfoElt.first);
        if (folder) {
            localFoldersSize += OCC::Utility::folderSize(folder->path());
            localFoldersDiskSize += OCC::Utility::folderDiskSize(folder->path());
        }
    }

    qint64 diskSpaceMissing =  localFoldersSize - localFoldersDiskSize - freeSize;
    bool diskSpaceWarning = diskSpaceMissing > 0;

    CustomMessageBox *msgBox = new CustomMessageBox(
                QMessageBox::Question,
                tr("Do you really want to turn off Lite Sync?"),
                diskSpaceWarning
                ? tr("You don't have enough space to sync all the files on your kDrive (%1 missing)."
                     " If you turn off Lite Sync, you need to select which folders to sync on your computer."
                     " In the meantime, the synchronization of your kDrive will be paused.")
                  .arg(OCC::Utility::octetsToString(diskSpaceMissing))
                : tr("If you turn off Lite Sync, all files will sync locally on your computer."),
                diskSpaceWarning,
                QMessageBox::NoButton, this);
    msgBox->addButton(tr("CONFIRM"), QMessageBox::Yes);
    msgBox->addButton(tr("CANCEL"), QMessageBox::No);
    msgBox->setDefaultButton(QMessageBox::Yes);
    int result = msgBox->exec();
    callback(result == QMessageBox::Yes, diskSpaceWarning);
}

void DrivePreferencesWidget::switchVfsOn(OCC::Folder *folder, std::shared_ptr<QMetaObject::Connection> connection)
{
    if (*connection) {
        QObject::disconnect(*connection);
    }

    // Wipe selective sync blacklist
    /*bool ok = false;
    auto oldBlacklist = folder->journalDb()->getSelectiveSyncList(OCC::SyncJournalDb::SelectiveSyncBlackList, &ok);
    folder->journalDb()->setSelectiveSyncList(OCC::SyncJournalDb::SelectiveSyncBlackList, {});*/

    // Change the folder vfs mode and load the plugin
    folder->setSupportsVirtualFiles(true);
    folder->setVfsOnOffSwitchPending(false);

    // Setting to Unspecified retains existing data.
    // Selective sync excluded folders become OnlineOnly.
    folder->setRootPinState(OCC::PinState::Unspecified);
    /*for (const auto &entry : oldBlacklist) {
        folder->journalDb()->schedulePathForRemoteDiscovery(entry);
        folder->vfs().setPinState(entry, OCC::PinState::OnlineOnly);
    }*/
    folder->slotNextSyncFullLocalDiscovery();

    OCC::FolderMan::instance()->scheduleFolder(folder);

    updateSmartSyncSwitchState();
}

void DrivePreferencesWidget::switchVfsOff(OCC::Folder *folder, bool diskSpaceWarning, std::shared_ptr<QMetaObject::Connection> connection)
{
    if (*connection) {
        QObject::disconnect(*connection);
    }

    // Also wipes virtual files, schedules remote discovery
    folder->setSupportsVirtualFiles(false);
    folder->setVfsOnOffSwitchPending(false);

    // Wipe pin states and selective sync db
    folder->setRootPinState(OCC::PinState::AlwaysLocal);
    //folder->journalDb()->setSelectiveSyncList(OCC::SyncJournalDb::SelectiveSyncBlackList, {});

    // Prevent issues with missing local files
    folder->slotNextSyncFullLocalDiscovery();

    if (diskSpaceWarning) {
        // Pause sync if disk space warning
        OCC::Utility::pauseSync(_accountId, QString(), true);
    }
    else {
        OCC::FolderMan::instance()->scheduleFolder(folder);
    }

    updateSmartSyncSwitchState();
}

void DrivePreferencesWidget::resetFoldersBlocs()
{
    QList<PreferencesBlocWidget *> folderBlocList = findChildren<PreferencesBlocWidget *>(folderBlocName);
    for (PreferencesBlocWidget *folderBloc : folderBlocList) {
        delete folderBloc;
    }

    update();
}

void DrivePreferencesWidget::updateFoldersBlocs()
{
    if (_accountInfo) {
        int foldersNextBeginIndex = _foldersBeginIndex;
        QList<QString> folderIdList = QList<QString>();
        QList<PreferencesBlocWidget *> folderBlocList = findChildren<PreferencesBlocWidget *>(folderBlocName);
        for (PreferencesBlocWidget *folderBloc : folderBlocList) {
            FolderItemWidget *folderItemWidget = folderBloc->findChild<FolderItemWidget *>();
            if (folderItemWidget) {
                auto folderInfoIt = _accountInfo->_folderMap.find(folderItemWidget->folderId());
                if (folderInfoIt == _accountInfo->_folderMap.end()) {
                    // Delete bloc when folder doesn't exist anymore
                    folderBloc->deleteLater();
                }
                else {
                    // Update folder widget
                    folderItemWidget->updateItem(folderInfoIt->second.get());
                    folderIdList << folderInfoIt->first;
                    int index = _mainVBox->indexOf(folderBloc) + 1;
                    if (foldersNextBeginIndex < index) {
                        foldersNextBeginIndex = index;
                    }
                }
            }
            else {
                qCDebug(lcDrivePreferencesWidget) << "Empty folder bloc!";
                Q_ASSERT(false);
            }
        }

        bool smartSync = isSmartSyncActivated();
        for (auto folderInfoElt : _accountInfo->_folderMap) {
            if (!folderIdList.contains(folderInfoElt.first)) {
                // Create folder bloc
                PreferencesBlocWidget *folderBloc = new PreferencesBlocWidget(this);
                folderBloc->setObjectName(folderBlocName);
                _mainVBox->insertWidget(foldersNextBeginIndex, folderBloc);

                QBoxLayout *folderBox = folderBloc->addLayout(QBoxLayout::Direction::LeftToRight);

                FolderItemWidget *folderItemWidget = new FolderItemWidget(folderInfoElt.first, folderInfoElt.second.get(), this);
                OCC::Folder *folder = OCC::FolderMan::instance()->folder(folderInfoElt.first);
                folderItemWidget->setSmartSync(smartSync);
                folderItemWidget->setFolderCompatibleWithSmartSync(folder->canSupportVirtualFiles());
                folderBox->addWidget(folderItemWidget);

                QFrame *line = folderBloc->addSeparator();
                line->setVisible(false);

                // Folder tree
                QBoxLayout *folderTreeBox = folderBloc->addLayout(QBoxLayout::Direction::LeftToRight, true);

                FolderTreeItemWidget *folderTreeItemWidget = new FolderTreeItemWidget(folderInfoElt.first, false, this);
                folderTreeItemWidget->setObjectName("updateFolderTreeItemWidget");
                folderTreeItemWidget->setVisible(false);
                folderTreeBox->addWidget(folderTreeItemWidget);

                connect(folderItemWidget, &FolderItemWidget::runSync, this, &DrivePreferencesWidget::onSyncTriggered);
                connect(folderItemWidget, &FolderItemWidget::pauseSync, this, &DrivePreferencesWidget::onPauseTriggered);
                connect(folderItemWidget, &FolderItemWidget::resumeSync, this, &DrivePreferencesWidget::onResumeTriggered);
                connect(folderItemWidget, &FolderItemWidget::unSync, this, &DrivePreferencesWidget::onUnsyncTriggered);
                connect(folderItemWidget, &FolderItemWidget::displayFolderDetail, this, &DrivePreferencesWidget::onDisplayFolderDetail);
                connect(folderItemWidget, &FolderItemWidget::openFolder, this, &DrivePreferencesWidget::onOpenFolder);
                connect(folderItemWidget, &FolderItemWidget::cancelUpdate, this, &DrivePreferencesWidget::onCancelUpdate);
                connect(folderItemWidget, &FolderItemWidget::validateUpdate, this, &DrivePreferencesWidget::onValidateUpdate);
                connect(folderTreeItemWidget, &FolderTreeItemWidget::terminated, this, &DrivePreferencesWidget:: onSubfoldersLoaded);
                connect(folderTreeItemWidget, &FolderTreeItemWidget::needToSave, this, &DrivePreferencesWidget::onNeedToSave);
            }
        }

        update();
    }
}

void DrivePreferencesWidget::refreshFoldersBlocs()
{
    QList<PreferencesBlocWidget *> folderBlocList = findChildren<PreferencesBlocWidget *>(folderBlocName);
    for (PreferencesBlocWidget *folderBloc : folderBlocList) {
        FolderTreeItemWidget *treeItemWidget = blocTreeItemWidget(folderBloc);
        if (treeItemWidget && treeItemWidget->isVisible()) {
            treeItemWidget->loadSubFolders();
        }
    }
}

DrivePreferencesWidget::JobResult DrivePreferencesWidget::folderHasSubfolders(const QString &folderPath)
{
    JobResult jobResult;
    OCC::AccountPtr accountPtr = OCC::AccountManager::instance()->getAccountFromId(_accountId);
    OCC::LsColJob *job;
    job = new OCC::LsColJob(accountPtr, folderPath, this);
    job->setProperties(QList<QByteArray>()
                       << "resourcetype"
                       << "http://owncloud.org/ns:size");

    connect(job, &OCC::LsColJob::directoryListingSubfolders, this, [&](QStringList list)
    {
        qCDebug(lcDrivePreferencesWidget) << "LsColJob request finished";
        if (list.size() <= 1) {
            emit jobTerminated(JobResult::No);
        }
        else {
            emit jobTerminated(JobResult::Yes);
        }
    });

    connect(job, &OCC::LsColJob::finishedWithError, this, [&](QNetworkReply *reply)
    {
        if (reply->error() == QNetworkReply::ContentNotFoundError) {
            qCDebug(lcDrivePreferencesWidget) << "LsColJob request finished";
            emit jobTerminated(JobResult::No);
        }
        else {
            qCDebug(lcDrivePreferencesWidget) << "LsColJob request failed: " << reply->error();
            emit jobTerminated(JobResult::Error);
        }
    });

    QEventLoop loop;
    loop.connect(this, &DrivePreferencesWidget::jobTerminated, &loop, [&](JobResult result)
    {
        jobResult = result;
        loop.quit();
    });
    job->start();
    loop.exec();

    return jobResult;
}

DrivePreferencesWidget::JobResult DrivePreferencesWidget::createFolder(const QString &folderPath)
{
    JobResult jobResult;
    OCC::AccountPtr accountPtr = OCC::AccountManager::instance()->getAccountFromId(_accountId);
    OCC::MkColJob *job = new OCC::MkColJob(accountPtr, folderPath, this);

    connect(job, static_cast<void (OCC::MkColJob::*)(QNetworkReply::NetworkError)>(&OCC::MkColJob::finished), this, [&](QNetworkReply::NetworkError error)
    {
        if (error == QNetworkReply::NoError) {
            qCDebug(lcDrivePreferencesWidget) << "MkColJob request finished";
            emit jobTerminated(JobResult::Yes);
        }
        else {
            qCDebug(lcDrivePreferencesWidget) << "MkColJob request failed: " << error;
            emit jobTerminated(JobResult::Error);
        }
    });

    connect(job, &OCC::AbstractNetworkJob::networkError, this, [&](QNetworkReply *reply)
    {
        qCDebug(lcDrivePreferencesWidget) << "MkColJob request failed: " << reply->error();
        emit jobTerminated(JobResult::Error);
    });

    QEventLoop loop;
    loop.connect(this, &DrivePreferencesWidget::jobTerminated, &loop, [&](JobResult result)
    {
        jobResult = result;
        loop.quit();
    });
    job->start();
    loop.exec();

    return jobResult;
}

FolderTreeItemWidget *DrivePreferencesWidget::blocTreeItemWidget(PreferencesBlocWidget *folderBloc)
{
    ASSERT(folderBloc)
    FolderTreeItemWidget *folderTreeItemWidget = folderBloc->findChild<FolderTreeItemWidget *>();
    if (!folderTreeItemWidget) {
        qCDebug(lcDrivePreferencesWidget) << "Bad folder bloc!";
        ASSERT(false);
        return nullptr;
    }

    return folderTreeItemWidget;
}

FolderItemWidget *DrivePreferencesWidget::blocItemWidget(PreferencesBlocWidget *folderBloc)
{
    ASSERT(folderBloc)
    FolderItemWidget *folderItemWidget = folderBloc->findChild<FolderItemWidget *>();
    if (!folderItemWidget) {
        qCDebug(lcDrivePreferencesWidget) << "Bad folder bloc!";
        ASSERT(false);
        return nullptr;
    }

    return folderItemWidget;
}

QFrame *DrivePreferencesWidget::blocSeparatorFrame(PreferencesBlocWidget *folderBloc)
{
    ASSERT(folderBloc)
    QFrame *separatorFrame = folderBloc->findChild<QFrame *>();
    if (!separatorFrame) {
        qCDebug(lcDrivePreferencesWidget) << "Bad folder bloc!";
        ASSERT(false);
        return nullptr;
    }

    return separatorFrame;
}

bool DrivePreferencesWidget::createMissingFolders(const QString &folderBasePath, const QString &folderPath)
{
    if (!folderPath.startsWith(folderBasePath)) {
        qCDebug(lcDrivePreferencesWidget) << "Bad folders: " << folderBasePath << " - " << folderPath;
        return false;
    }

    // Create server folders
    QStringList folderBasePaths = folderBasePath.split(dirSeparator);
    int folderBasePathsCount = folderBasePaths.size();
    QStringList folderPaths = folderPath.split(dirSeparator);
    int folderPathsCount = folderPaths.size();
    QString folderToCreate(folderBasePath);
    for (int i = folderBasePathsCount; i < folderPathsCount; i++) {
        folderToCreate += dirSeparator + folderPaths[i];
        if (createFolder(folderToCreate) == JobResult::Yes) {
            qCDebug(lcDrivePreferencesWidget) << "Folder created: " << folderToCreate;
        }
        else {
            qCDebug(lcDrivePreferencesWidget) << "Folder creation error: " << folderToCreate;
            return false;
        }
    }

    return true;
}

bool DrivePreferencesWidget::addSynchronization(const QString &localFolderPath, bool smartSync, const QString &serverFolderPath, QStringList blackList)
{
    qCInfo(lcDrivePreferencesWidget) << "Adding folder definition for" << localFolderPath << serverFolderPath;

    OCC::FolderDefinition folderDefinition;
    folderDefinition.localPath = localFolderPath;
    folderDefinition.targetPath = OCC::FolderDefinition::prepareTargetPath(serverFolderPath);
    folderDefinition.ignoreHiddenFiles = OCC::FolderMan::instance()->ignoreHiddenFiles();
    if (smartSync) {
        folderDefinition.virtualFilesMode = OCC::bestAvailableVfsMode(OCC::ConfigFile().showExperimentalOptions());
    }
#ifdef Q_OS_WIN
    if (OCC::FolderMan::instance()->navigationPaneHelper().showInExplorerNavigationPane()) {
        folderDefinition.navigationPaneClsid = QUuid::createUuid();
    }
#endif

    OCC::AccountStatePtr accountStatePtr = OCC::AccountManager::instance()->getAccountStateFromId(_accountId);
    OCC::Folder *folder = OCC::FolderMan::instance()->addFolder(accountStatePtr.data(), folderDefinition);
    if (folder) {
        if (folderDefinition.virtualFilesMode != OCC::Vfs::Off && smartSync) {
            folder->setRootPinState(OCC::PinState::OnlineOnly);
        }

        folder->journalDb()->setSelectiveSyncList(OCC::SyncJournalDb::SelectiveSyncBlackList, blackList);
        OCC::ConfigFile cfg;
        if (!cfg.newBigFolderSizeLimit().first) {
            folder->journalDb()->setSelectiveSyncList(OCC::SyncJournalDb::SelectiveSyncWhiteList, QStringList() << dirSeparator);
        }
    }

    return true;
}

bool DrivePreferencesWidget::updateSelectiveSyncList(OCC::Folder *folder)
{
    bool ok;
    QStringList undecidedList = folder->journalDb()->getSelectiveSyncList(OCC::SyncJournalDb::SelectiveSyncUndecidedList, &ok);
    if (!ok) {
        qCWarning(lcDrivePreferencesWidget) << "Could not read selective sync list from db.";
        return false;
    }

    // If this folder had no undecided entries, skip it.
    if (undecidedList.isEmpty()) {
        return true;
    }

    // Remove all undecided folders from the blacklist
    QStringList blackList = folder->journalDb()->getSelectiveSyncList(OCC::SyncJournalDb::SelectiveSyncBlackList, &ok);
    if (!ok) {
        qCWarning(lcDrivePreferencesWidget) << "Could not read selective sync list from db.";
        return false;
    }
    foreach (const auto &undecidedFolder, undecidedList) {
        blackList.removeAll(undecidedFolder);
    }
    folder->journalDb()->setSelectiveSyncList(OCC::SyncJournalDb::SelectiveSyncBlackList, blackList);

    // Add all undecided folders to the white list
    QStringList whiteList = folder->journalDb()->getSelectiveSyncList(OCC::SyncJournalDb::SelectiveSyncWhiteList, &ok);
    if (!ok) {
        qCWarning(lcDrivePreferencesWidget) << "Could not read selective sync list from db.";
        return false;
    }
    whiteList += undecidedList;
    folder->journalDb()->setSelectiveSyncList(OCC::SyncJournalDb::SelectiveSyncWhiteList, whiteList);

    // Clear the undecided list
    folder->journalDb()->setSelectiveSyncList(OCC::SyncJournalDb::SelectiveSyncUndecidedList, QStringList());

    // Trigger a sync
    if (folder->isBusy()) {
        folder->slotTerminateSync();
    }

    // The part that changed should not be read from the DB on next sync because there might be new folders
    // (the ones that are no longer in the blacklist)
    foreach (const auto &it, undecidedList) {
        folder->journalDb()->schedulePathForRemoteDiscovery(it);
        folder->schedulePathForLocalDiscovery(it);
    }

    // Also make sure we see the local file that had been ignored before
    folder->slotNextSyncFullLocalDiscovery();
    OCC::FolderMan::instance()->scheduleFolder(folder);

    return true;
}

bool DrivePreferencesWidget::isSmartSyncActivated()
{
    // Check if smart sync is activated at least for one folder
    bool smartSync = false;
    for (auto folderInfoElt : _accountInfo->_folderMap) {
        OCC::Folder *folder = OCC::FolderMan::instance()->folder(folderInfoElt.first);
        if (folder && folder->vfs().mode() != OCC::Vfs::Off) {
            smartSync = true;
            break;
        }
    }

    return smartSync;
}

void DrivePreferencesWidget::onLinkActivated(const QString &link)
{
    if (link == OCC::Utility::learnMoreLink) {
        // Learn more: Lite Sync
        if (!QDesktopServices::openUrl(QUrl(LEARNMORE_LITESYNC_URL))) {
            qCWarning(lcDrivePreferencesWidget) << "QDesktopServices::openUrl failed for " << link;
            CustomMessageBox *msgBox = new CustomMessageBox(
                        QMessageBox::Warning,
                        tr("Unable to open link %1.").arg(link),
                        QMessageBox::Ok, this);
            msgBox->exec();
        }
    }
}

void DrivePreferencesWidget::onErrorsWidgetClicked()
{
    emit displayErrors(_accountId);
}

void DrivePreferencesWidget::onBigFoldersWarningWidgetClicked()
{
    QStringList accountUndecidedList;
    if (_accountInfo) {
        for (auto folderInfoElt : _accountInfo->_folderMap) {
            OCC::Folder *folder = OCC::FolderMan::instance()->folder(folderInfoElt.first);
            if (folder) {
                bool ok;
                accountUndecidedList << folder->journalDb()->getSelectiveSyncList(OCC::SyncJournalDb::SelectiveSyncUndecidedList, &ok);
            }
        }
    }

    BigFoldersDialog *dialog = new BigFoldersDialog(accountUndecidedList, _accountInfo, this);
    if (dialog->exec() == QDialog::Accepted) {
        for (auto folderInfoElt : _accountInfo->_folderMap) {
            OCC::Folder *folder = OCC::FolderMan::instance()->folder(folderInfoElt.first);
            if (folder) {
                if (!updateSelectiveSyncList(folder)) {
                    qCWarning(lcDrivePreferencesWidget) << "updateSelectiveSyncList error";
                    break;
                }
            }
        }

        _displayBigFoldersWarningWidget->setVisible(existUndecidedList());
        refreshFoldersBlocs();
    }
}

void DrivePreferencesWidget::onAddFolder(bool checked)
{
    Q_UNUSED(checked)

    const QString addFolderError = tr("New folder synchronization failed!");
    QString localFolderPath = QString();
    bool smartSync = false;
    QString serverFolderPath = QString();
    QString serverFolderBasePath = QString();
    JobResult folderHasSubfoldersJobResult = JobResult::No;
    QStringList blackList = QStringList();
    QString localFolderName = QString();
    qint64 localFolderSize = 0;
    QString serverFolderName = QString();
    qint64 serverFolderSize = 0;
    AddFolderStep nextStep = SelectLocalFolder;

    while (true) {
        if (nextStep == SelectLocalFolder) {
            LocalFolderDialog *localFolderDialog = new LocalFolderDialog(localFolderPath, this);
            localFolderDialog->setSmartSync(isSmartSyncActivated());
            connect(localFolderDialog, &LocalFolderDialog::openFolder, this, &DrivePreferencesWidget::onOpenFolder);
            if (localFolderDialog->exec(OCC::Utility::getTopLevelWidget(this)->pos()) == QDialog::Rejected) {
                break;
            }

            localFolderPath = localFolderDialog->localFolderPath();
            QFileInfo localFolderInfo(localFolderPath);
            localFolderName = localFolderInfo.baseName();
            localFolderSize = OCC::Utility::folderSize(localFolderPath);
            smartSync = localFolderDialog->folderCompatibleWithSmartSync();
            qCDebug(lcDrivePreferencesWidget) << "Local folder selected: " << localFolderPath;
            nextStep = SelectServerBaseFolder;
        }

        if (nextStep == SelectServerBaseFolder) {
            ServerBaseFolderDialog *serverBaseFolderDialog = new ServerBaseFolderDialog(_accountId, localFolderName, this);
            int ret = serverBaseFolderDialog->exec(OCC::Utility::getTopLevelWidget(this)->pos());
            if (ret == QDialog::Rejected) {
                qCDebug(lcDrivePreferencesWidget) << "Cancel: " << nextStep;
                break;
            }
            else if (ret == -1) {
                // Go back
                nextStep = SelectLocalFolder;
                continue;
            }

            serverFolderPath = serverBaseFolderDialog->serverFolderPath();
            serverFolderBasePath = serverBaseFolderDialog->serverFolderBasePath();
            serverFolderSize = serverBaseFolderDialog->selectionSize();
            if (serverFolderPath.isEmpty()) {
                serverFolderName = _accountInfo->_name;
            }
            else {
                QDir serverFolderDir(serverFolderPath);
                serverFolderName = serverFolderDir.dirName();
            }
            qCDebug(lcDrivePreferencesWidget) << "Server folder selected: " << serverFolderPath;
            nextStep = SelectServerFolders;
        }

        if (nextStep == SelectServerFolders) {
            folderHasSubfoldersJobResult = folderHasSubfolders(serverFolderPath);
            if (folderHasSubfoldersJobResult == JobResult::Error) {
                CustomMessageBox *msgBox = new CustomMessageBox(
                            QMessageBox::Warning,
                            addFolderError,
                            QMessageBox::Ok, this);
                msgBox->setDefaultButton(QMessageBox::Ok);
                msgBox->exec();
                break;
            }

            if (folderHasSubfoldersJobResult == JobResult::Yes) {
                ServerFoldersDialog *serverFoldersDialog = new ServerFoldersDialog(_accountId, serverFolderName, serverFolderPath, this);
                int ret = serverFoldersDialog->exec(OCC::Utility::getTopLevelWidget(this)->pos());
                if (ret == QDialog::Rejected) {
                    qCDebug(lcDrivePreferencesWidget) << "Cancel: " << nextStep;
                    break;
                }
                else if (ret == -1) {
                    // Go back
                    nextStep = SelectServerBaseFolder;
                    continue;
                }

                serverFolderSize = serverFoldersDialog->selectionSize();
                blackList = serverFoldersDialog->createBlackList();
                qCDebug(lcDrivePreferencesWidget) << "Server subfolders selected";
            }
            nextStep = Confirm;
        }

        if (nextStep == Confirm) {
            ConfirmSynchronizationDialog *confirmSynchronizationDialog = new ConfirmSynchronizationDialog(
                        localFolderName, localFolderSize, serverFolderName, serverFolderSize, this);
            int ret = confirmSynchronizationDialog->exec(OCC::Utility::getTopLevelWidget(this)->pos());
            if (ret == QDialog::Rejected) {
                qCDebug(lcDrivePreferencesWidget) << "Cancel: " << nextStep;
                break;
            }
            else if (ret == -1) {
                // Go back
                nextStep = folderHasSubfoldersJobResult == JobResult::Yes ? SelectServerFolders : SelectServerBaseFolder;
                continue;
            }

            // Setup local folder
            const QDir localFolderDir(localFolderPath);
            if (localFolderDir.exists()) {
                OCC::FileSystem::setFolderMinimumPermissions(localFolderPath);
                OCC::Utility::setupFavLink(localFolderPath);
                qCDebug(lcDrivePreferencesWidget) << "Local folder setup: " << localFolderPath;
            }
            else {
                qCDebug(lcDrivePreferencesWidget) << "Local folder doesn't exist anymore: " << localFolderPath;
                CustomMessageBox *msgBox = new CustomMessageBox(
                            QMessageBox::Warning,
                            addFolderError,
                            QMessageBox::Ok, this);
                msgBox->setDefaultButton(QMessageBox::Ok);
                msgBox->exec();
                break;
            }

            if (serverFolderPath != serverFolderBasePath) {
                // Create missing server folders
                if (!createMissingFolders(serverFolderBasePath, serverFolderPath)) {
                    CustomMessageBox *msgBox = new CustomMessageBox(
                                QMessageBox::Warning,
                                addFolderError,
                                QMessageBox::Ok, this);
                    msgBox->setDefaultButton(QMessageBox::Ok);
                    msgBox->exec();
                    break;
                }
            }

            // Add folder to synchronization
            if (!addSynchronization(localFolderPath, smartSync, serverFolderPath, blackList)) {
                CustomMessageBox *msgBox = new CustomMessageBox(
                            QMessageBox::Warning,
                            addFolderError,
                            QMessageBox::Ok, this);
                msgBox->setDefaultButton(QMessageBox::Ok);
                msgBox->exec();
            }

            break;
        }
    }
}

void DrivePreferencesWidget::onSmartSyncSwitchClicked(bool checked)
{
    if (checked) {
        askEnableSmartSync([this](bool enable) {
            if (!enable) {
                _smartSyncSwitch->setCheckState(Qt::Unchecked);
                return;
            }

            _smartSyncSwitch->setEnabled(false);
            _smartSyncSwitch->setToolTip(tr("Lite Sync activation in progress"));
            for (auto folderInfoElt : _accountInfo->_folderMap) {
                OCC::Folder *folder = OCC::FolderMan::instance()->folder(folderInfoElt.first);
                if (folder && folder->canSupportVirtualFiles()) {
                    // It is unsafe to switch on vfs while a sync is running - wait if necessary.
                    auto connection = std::make_shared<QMetaObject::Connection>();
                    if (folder->isSyncRunning()) {
                        folder->setVfsOnOffSwitchPending(true);
                        *connection = connect(folder, &OCC::Folder::syncFinished,
                                              this, [=](){ switchVfsOn(folder, connection); });
                        folder->slotTerminateSync();
                    } else {
                        switchVfsOn(folder, connection);
                    }
                }
            }
        });
    }
    else {
        askDisableSmartSync([this](bool enable, bool diskSpaceWarning) {
            if (!enable) {
                _smartSyncSwitch->setCheckState(Qt::Checked);
                return;
            }

            _smartSyncSwitch->setEnabled(false);
            _smartSyncSwitch->setToolTip(tr("Lite Sync deactivation in progress"));
            for (auto folderInfoElt : _accountInfo->_folderMap) {
                OCC::Folder *folder = OCC::FolderMan::instance()->folder(folderInfoElt.first);
                if (folder) {
                    // It is unsafe to switch off vfs while a sync is running - wait if necessary.
                    auto connection = std::make_shared<QMetaObject::Connection>();
                    if (folder->isSyncRunning()) {
                        folder->setVfsOnOffSwitchPending(true);
                        *connection = connect(folder, &OCC::Folder::syncFinished,
                                              this, [=](){ switchVfsOff(folder, diskSpaceWarning, connection); });
                        folder->slotTerminateSync();
                    } else {
                        switchVfsOff(folder, diskSpaceWarning, connection);
                    }
                }
            }
        });
    }
}

void DrivePreferencesWidget::onNotificationsSwitchClicked(bool checked)
{
    OCC::FolderMan::instance()->setNotificationsDisabled(_accountId, !checked);
}

void DrivePreferencesWidget::onErrorAdded()
{
    _displayErrorsWidget->setVisible(true);
}

void DrivePreferencesWidget::onRemoveDrive(bool checked)
{
    Q_UNUSED(checked)

    emit removeDrive(_accountId);
}

void DrivePreferencesWidget::onSyncTriggered(const QString &folderId)
{
    OCC::Utility::runSync(_accountId, folderId);
}

void DrivePreferencesWidget::onPauseTriggered(const QString &folderId)
{
    OCC::Utility::pauseSync(_accountId, folderId, true);
}

void DrivePreferencesWidget::onResumeTriggered(const QString &folderId)
{
    OCC::Utility::pauseSync(_accountId, folderId, false);
}

void DrivePreferencesWidget::onUnsyncTriggered(const QString &folderId)
{
    OCC::FolderMan *folderMan = OCC::FolderMan::instance();
    auto folder = folderMan->folder(folderId);

    CustomMessageBox *msgBox = new CustomMessageBox(
                QMessageBox::Question,
                tr("Do you really want to stop syncing the folder <i>%1</i> ?<br>"
                   "<b>Note:</b> This will <b>not</b> delete any files.")
                .arg(folder->path()),
                QMessageBox::NoButton, this);
    msgBox->addButton(tr("REMOVE FOLDER SYNC CONNECTION"), QMessageBox::Yes);
    msgBox->addButton(tr("CANCEL"), QMessageBox::No);
    msgBox->setDefaultButton(QMessageBox::No);
    int ret = msgBox->exec();
    if (ret != QDialog::Rejected) {
        if (ret == QMessageBox::Yes) {
            PreferencesBlocWidget *folderBloc = (PreferencesBlocWidget *) sender()->parent();
            ASSERT(folderBloc)

            // Remove folder
            folderBloc->setVisible(false);
            folderMan->removeFolder(folder);
        }
    }
}

void DrivePreferencesWidget::onDisplayFolderDetail(const QString &folderId, bool display)
{
    if (_accountInfo) {
        FolderTreeItemWidget *treeItemWidget = blocTreeItemWidget((PreferencesBlocWidget *) sender()->parent());
        ASSERT(treeItemWidget)
        ASSERT(treeItemWidget->folderId() == folderId)

        if (display) {
            setCursor(Qt::WaitCursor);
            treeItemWidget->loadSubFolders();
        }
        else {
            QFrame *separatorFrame = blocSeparatorFrame((PreferencesBlocWidget *) sender()->parent());
            ASSERT(separatorFrame)

            treeItemWidget->setVisible(false);
            separatorFrame->setVisible(false);
        }
    }
}

void DrivePreferencesWidget::onOpenFolder(const QString &filePath)
{
    emit openFolder(filePath);
}

void DrivePreferencesWidget::onSubfoldersLoaded(bool error, bool empty)
{
    setCursor(Qt::ArrowCursor);
    if (error || empty) {
        FolderItemWidget *itemWidget = blocItemWidget((PreferencesBlocWidget *) sender()->parent());
        ASSERT(itemWidget)
        emit itemWidget->displayFolderDetailCanceled();

        if (error) {
            CustomMessageBox *msgBox = new CustomMessageBox(
                        QMessageBox::Warning,
                        tr("An error occurred while loading the list of sub folders."),
                        QMessageBox::Ok, this);
            msgBox->setDefaultButton(QMessageBox::Ok);
            msgBox->exec();
        }
        else if (empty) {
            CustomMessageBox *msgBox = new CustomMessageBox(
                        QMessageBox::Information,
                        tr("No subfolders currently on the server."),
                        QMessageBox::Ok, this);
            msgBox->setDefaultButton(QMessageBox::Ok);
            msgBox->exec();
        }
        emit
    }
    else {
        FolderTreeItemWidget *treeItemWidget = (FolderTreeItemWidget *) sender();
        ASSERT(treeItemWidget)

        QFrame *separatorFrame = blocSeparatorFrame((PreferencesBlocWidget *) sender()->parent());
        ASSERT(separatorFrame)

        treeItemWidget->setVisible(true);
        separatorFrame->setVisible(true);
    }
}

void DrivePreferencesWidget::onNeedToSave()
{
    // Show update widget
    FolderItemWidget *itemWidget = blocItemWidget((PreferencesBlocWidget *) sender()->parent());
    if (itemWidget) {
        itemWidget->setUpdateWidgetVisible(true);
    }
}

void DrivePreferencesWidget::onCancelUpdate(const QString &folderId)
{
    FolderTreeItemWidget *treeItemWidget = blocTreeItemWidget((PreferencesBlocWidget *) sender()->parent());
    if (treeItemWidget) {
        ASSERT(treeItemWidget->folderId() == folderId);
        treeItemWidget->loadSubFolders();

        // Hide update widget
        FolderItemWidget *itemWidget = (FolderItemWidget *) sender();
        if (itemWidget) {
            itemWidget->setUpdateWidgetVisible(false);
        }
    }
}

void DrivePreferencesWidget::onValidateUpdate(const QString &folderId)
{
    FolderTreeItemWidget *treeItemWidget = blocTreeItemWidget((PreferencesBlocWidget *) sender()->parent());
    if (treeItemWidget) {
        ASSERT(treeItemWidget->folderId() == folderId);
        OCC::Folder *folder = OCC::FolderMan::instance()->folder(treeItemWidget->folderId());
        if (folder) {
            // Clear the undecided list
            folder->journalDb()->setSelectiveSyncList(OCC::SyncJournalDb::SelectiveSyncUndecidedList, QStringList());
            _displayBigFoldersWarningWidget->setVisible(false);

            // Update the black list
            bool ok;
            QStringList oldBlackList = folder->journalDb()->getSelectiveSyncList(OCC::SyncJournalDb::SelectiveSyncBlackList, &ok);
            if (!ok) {
                qCWarning(lcDrivePreferencesWidget) << "Could not read selective sync list from db.";
                return;
            }
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
            QSet<QString> oldBlackListSet(oldBlackList.begin(), oldBlackList.end());
#else
            QSet<QString> oldBlackListSet = oldBlackList.toSet();
#endif

            QStringList blackList = treeItemWidget->createBlackList();
            folder->journalDb()->setSelectiveSyncList(OCC::SyncJournalDb::SelectiveSyncBlackList, blackList);

            if (folder->isBusy()) {
                folder->slotTerminateSync();
            }

            // The part that changed should not be read from the DB on next sync because there might be new folders
            // (the ones that are no longer in the blacklist)
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
            QSet<QString> blackListSet(blackList.begin(), blackList.end());
#else
            QSet<QString> blackListSet = blackList.toSet();
#endif
            QSet<QString> changes = (oldBlackListSet - blackListSet) + (blackListSet - oldBlackListSet);
            foreach (const auto &it, changes) {
                folder->journalDb()->schedulePathForRemoteDiscovery(it);
                folder->schedulePathForLocalDiscovery(it);
            }
            // Also make sure we see the local file that had been ignored before
            folder->slotNextSyncFullLocalDiscovery();

            OCC::FolderMan::instance()->scheduleFolder(folder);

            // Hide update widget
            FolderItemWidget *itemWidget = (FolderItemWidget *) sender();
            if (itemWidget) {
                itemWidget->setUpdateWidgetVisible(false);
            }
        }
    }
}

void DrivePreferencesWidget::onNewBigFolderDiscovered(const QString &path)
{
    Q_UNUSED(path)

    _displayBigFoldersWarningWidget->setVisible(existUndecidedList());
    refreshFoldersBlocs();
}

void DrivePreferencesWidget::onUndecidedListsCleared()
{
    _displayBigFoldersWarningWidget->setVisible(existUndecidedList());
    refreshFoldersBlocs();
}

}
