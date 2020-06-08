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
#include "custommessagebox.h"
#include "custompushbutton.h"
#include "accountmanager.h"
#include "configfile.h"
#include "guiutility.h"
#include "theme.h"
#include "common/utility.h"
#include "common/vfs.h"

#include <QDir>
#include <QIcon>
#include <QLabel>
#include <QScrollArea>
#include <QStandardPaths>
#include <QStyle>

namespace KDC {

static const int boxHMargin= 20;
static const int boxVMargin = 20;
static const int boxVSpacing = 12;
static const int textHSpacing = 5;
static const int avatarSize = 40;

static const QString folderBlocName("folderBloc");

Q_LOGGING_CATEGORY(lcDrivePreferencesWidget, "drivepreferenceswidget", QtInfoMsg)

DrivePreferencesWidget::DrivePreferencesWidget(QWidget *parent)
    : QWidget(parent)
    , _accountId(QString())
    , _accountInfo(nullptr)
    , _mainVBox(nullptr)
    , _displayErrorsWidget(nullptr)
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
     *      errorsWidget
     *      foldersLabel
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
    QLabel *synchronizationLabel = new QLabel(tr("Synchronization"), this);
    synchronizationLabel->setObjectName("blocLabel");
    _mainVBox->addWidget(synchronizationLabel);

    PreferencesBlocWidget *synchronizationBloc = new PreferencesBlocWidget(this);
    _mainVBox->addWidget(synchronizationBloc);

    // Smart sync
    if (OCC::Theme::instance()->showVirtualFilesOption() && OCC::bestAvailableVfsMode() != OCC::Vfs::Off) {
        QBoxLayout *smartSyncBox = synchronizationBloc->addLayout(QBoxLayout::Direction::TopToBottom);

        QHBoxLayout *smartSync1HBox = new QHBoxLayout();
        smartSync1HBox->setContentsMargins(0, 0, 0, 0);
        smartSync1HBox->setSpacing(0);
        smartSyncBox->addLayout(smartSync1HBox);

        QLabel *smartSyncLabel = new QLabel(tr("Activate smart synchronization"), this);
        smartSync1HBox->addWidget(smartSyncLabel);
        smartSync1HBox->addStretch();

        _smartSyncSwitch = new CustomSwitch(this);
        _smartSyncSwitch->setLayoutDirection(Qt::RightToLeft);
        _smartSyncSwitch->setAttribute(Qt::WA_MacShowFocusRect, false);
        smartSync1HBox->addWidget(_smartSyncSwitch);

        _smartSyncDescriptionLabel = new QLabel(this);
        _smartSyncDescriptionLabel->setObjectName("description");
        _smartSyncDescriptionLabel->setText(tr("Synchronize all your files without using your computer space."
                                              " <a style=\"%1\" href=\"ref\">Learn more</a>")
                                            .arg(OCC::Utility::linkStyle));
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

    QLabel *notificationsTitleLabel = new QLabel(tr("Disable the notifications"), this);
    notifications1HBox->addWidget(notificationsTitleLabel);

    _notificationsSwitch = new CustomSwitch(this);
    _notificationsSwitch->setLayoutDirection(Qt::RightToLeft);
    _notificationsSwitch->setAttribute(Qt::WA_MacShowFocusRect, false);
    _notificationsSwitch->setCheckState(cfg.monoIcons() ? Qt::Checked : Qt::Unchecked);
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
    connect(addFolderButton, &CustomPushButton::clicked, this, &DrivePreferencesWidget::onAddFolder);
    if (_smartSyncSwitch && _smartSyncDescriptionLabel) {
        connect(_smartSyncSwitch, &CustomSwitch::clicked, this, &DrivePreferencesWidget::onSmartSyncSwitchClicked);
        connect(_smartSyncDescriptionLabel, &QLabel::linkActivated, this, &DrivePreferencesWidget::onDisplaySmartSyncInfo);
    }
    connect(_notificationsSwitch, &CustomSwitch::clicked, this, &DrivePreferencesWidget::onNotificationsSwitchClicked);
    connect(this, &DrivePreferencesWidget::errorAdded, this, &DrivePreferencesWidget::onErrorAdded);
    connect(removeDriveButton, &CustomToolButton::clicked, this, &DrivePreferencesWidget::onRemoveDrive);
}

void DrivePreferencesWidget::setAccount(const QString &accountId, const AccountInfo *accountInfo, bool errors)
{
    if (_accountId != accountId) {
        reset();
    }

    _accountId = accountId;
    _accountInfo = accountInfo;
    _displayErrorsWidget->setVisible(errors);
    updateFoldersBlocs();
    updateSmartSyncSwitchState();
    _notificationsSwitch->setChecked(OCC::FolderMan::instance()->notificationsDisabled(_accountId));
    updateAccountInfo();
}

void DrivePreferencesWidget::reset()
{
    _accountId = QString();
    _accountInfo = nullptr;
    _displayErrorsWidget->setVisible(false);
    resetFoldersBlocs();
    _smartSyncSwitch->setChecked(false);
    _notificationsSwitch->setChecked(false);
    _accountAvatarLabel->setPixmap(QPixmap());
    _accountNameLabel->setText(QString());
    _accountMailLabel->setText(QString());
}

void DrivePreferencesWidget::updateSmartSyncSwitchState()
{
    if (_smartSyncSwitch) {
        bool smartSyncAvailable = false;
        bool oneDoesntSupportsVirtualFiles = false;
        bool oneHasVfsOnOffSwitchPending = false;
        for (auto folderInfoElt : _accountInfo->_folderMap) {
            OCC::Folder *folder = OCC::FolderMan::instance()->folder(folderInfoElt.first);
            if (folder) {
                oneDoesntSupportsVirtualFiles |= !folder->supportsVirtualFiles();
                oneHasVfsOnOffSwitchPending |= folder->isVfsOnOffSwitchPending();
            }
        }
        smartSyncAvailable = oneDoesntSupportsVirtualFiles && !oneHasVfsOnOffSwitchPending;
        _smartSyncSwitch->setCheckState(smartSyncAvailable ? Qt::Unchecked : Qt::Checked);
        _smartSyncSwitch->setEnabled(!oneHasVfsOnOffSwitchPending);
        if (!oneHasVfsOnOffSwitchPending) {
            _smartSyncSwitch->setToolTip("");
        }
    }
}

void DrivePreferencesWidget::updateAccountInfo()
{
    OCC::AccountPtr accountPtr = OCC::AccountManager::instance()->getAccountFromId(_accountId);
    if (!accountPtr.isNull()) {
        _accountAvatarLabel->setPixmap(OCC::Utility::getPixmapFromImage(accountPtr->avatar(),
                                                                        QSize(avatarSize, avatarSize)));

        _accountNameLabel->setText(accountPtr->davDisplayName());

        QString email = OCC::AccountManager::instance()->getUserId(accountPtr);
        _accountMailLabel->setText(email);
    }
    else {
        qCDebug(lcDrivePreferencesWidget) << "Null pointer!";
        Q_ASSERT(false);
    }
}

void DrivePreferencesWidget::askEnableSmartSync(const std::function<void (bool)> &callback)
{
    const auto bestVfsMode = OCC::bestAvailableVfsMode();
    CustomMessageBox *msgBox = nullptr;
    if (bestVfsMode == OCC::Vfs::WindowsCfApi) {
        msgBox = new CustomMessageBox(
                    QMessageBox::Warning,
                    tr("When the \"virtual files\" mode is enabled no files will be downloaded initially. "
                       "Instead a virtual file will be created for each file that exists on the server. "
                       "When a file is opened its contents will be downloaded automatically. "
                       "Alternatively, files can be downloaded manually by using their context menu.\n\n"
                       "The virtual files mode is mutually exclusive with selective sync. "
                       "Currently unselected folders will be translated to online-only folders "
                       "and your selective sync settings will be reset."),
                     QMessageBox::NoButton, this);
        msgBox->addButton(tr("Enable virtual files"), QMessageBox::Yes);
        msgBox->addButton(tr("Continue to use selective sync"), QMessageBox::No);
    } else {
        ASSERT(bestVfsMode == OCC::Vfs::WithSuffix)
        msgBox = new CustomMessageBox(
                    QMessageBox::Warning,
                    tr("When the \"virtual files\" mode is enabled no files will be downloaded initially. "
                       "Instead, a tiny \"%1\" file will be created for each file that exists on the server. "
                       "The contents can be downloaded by running these files or by using their context menu.\n\n"
                       "The virtual files mode is mutually exclusive with selective sync. "
                       "Currently unselected folders will be translated to online-only folders "
                       "and your selective sync settings will be reset.\n\n"
                       "Switching to this mode will abort any currently running synchronization.\n\n"
                       "This is a new, experimental mode. If you decide to use it, please report any "
                       "issues that come up.").arg(APPLICATION_DOTVIRTUALFILE_SUFFIX),
                    QMessageBox::NoButton, this);
        msgBox->addButton(tr("Enable experimental placeholder mode"), QMessageBox::Yes);
        msgBox->addButton(tr("Stay safe"), QMessageBox::No);
    }
    int result = msgBox->exec();
    callback(result == QMessageBox::Yes);
}

void DrivePreferencesWidget::askDisableSmartSync(const std::function<void (bool)> &callback)
{
    CustomMessageBox *msgBox = new CustomMessageBox(
                QMessageBox::Question,
                tr("This action will disable virtual file support. As a consequence contents of folders that "
                   "are currently marked as 'available online only' will be downloaded.\n\n"
                   "This action will abort any currently running synchronization."),
                QMessageBox::NoButton, this);
    msgBox->addButton(tr("Disable support"), QMessageBox::Yes);
    msgBox->addButton(tr("Cancel"), QMessageBox::No);
    int result = msgBox->exec();
    callback(result == QMessageBox::Yes);
}

void DrivePreferencesWidget::switchVfsOn(OCC::Folder *folder, std::shared_ptr<QMetaObject::Connection> connection)
{
    if (*connection) {
        QObject::disconnect(*connection);
    }

    // Wipe selective sync blacklist
    bool ok = false;
    auto oldBlacklist = folder->journalDb()->getSelectiveSyncList(OCC::SyncJournalDb::SelectiveSyncBlackList, &ok);
    folder->journalDb()->setSelectiveSyncList(OCC::SyncJournalDb::SelectiveSyncBlackList, {});

    // Change the folder vfs mode and load the plugin
    folder->setSupportsVirtualFiles(true);
    folder->setVfsOnOffSwitchPending(false);

    // Setting to Unspecified retains existing data.
    // Selective sync excluded folders become OnlineOnly.
    folder->setRootPinState(OCC::PinState::Unspecified);
    for (const auto &entry : oldBlacklist) {
        folder->journalDb()->schedulePathForRemoteDiscovery(entry);
        folder->vfs().setPinState(entry, OCC::PinState::OnlineOnly);
    }
    folder->slotNextSyncFullLocalDiscovery();

    OCC::FolderMan::instance()->scheduleFolder(folder);

    updateSmartSyncSwitchState();
}

void DrivePreferencesWidget::switchVfsOff(OCC::Folder *folder, std::shared_ptr<QMetaObject::Connection> connection)
{
    if (*connection) {
        QObject::disconnect(*connection);
    }

    // Also wipes virtual files, schedules remote discovery
    folder->setSupportsVirtualFiles(false);
    folder->setVfsOnOffSwitchPending(false);

    // Wipe pin states and selective sync db
    folder->setRootPinState(OCC::PinState::AlwaysLocal);
    folder->journalDb()->setSelectiveSyncList(OCC::SyncJournalDb::SelectiveSyncBlackList, {});

    // Prevent issues with missing local files
    folder->slotNextSyncFullLocalDiscovery();

    OCC::FolderMan::instance()->scheduleFolder(folder);

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
                    delete folderBloc;
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

        for (auto folderInfoElt : _accountInfo->_folderMap) {
            if (!folderIdList.contains(folderInfoElt.first)) {
                // Create folder bloc
                PreferencesBlocWidget *folderBloc = new PreferencesBlocWidget(this);
                folderBloc->setObjectName(folderBlocName);
                _mainVBox->insertWidget(foldersNextBeginIndex, folderBloc);

                QBoxLayout *folderBox = folderBloc->addLayout(QBoxLayout::Direction::LeftToRight);

                FolderItemWidget *folderItemWidget = new FolderItemWidget(folderInfoElt.first, folderInfoElt.second.get(), this);
                folderBox->addWidget(folderItemWidget);

                folderBloc->addSeparator();

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
                connect(folderTreeItemWidget, &FolderTreeItemWidget::message, this, &DrivePreferencesWidget::onDisplayMessage);
                connect(folderTreeItemWidget, &FolderTreeItemWidget::needToSave, this, &DrivePreferencesWidget::onNeedToSave);
            }
        }

        update();
    }
}

bool DrivePreferencesWidget::folderHasSubfolders(const QString &folderPath)
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

    return jobResult == JobResult::Yes;
}

bool DrivePreferencesWidget::createFolder(const QString &folderPath)
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

    return jobResult == JobResult::Yes;
}

FolderTreeItemWidget *DrivePreferencesWidget::folderTreeItemWidget(QObject *folderItemWidget)
{
    PreferencesBlocWidget *folderBloc = (PreferencesBlocWidget *) folderItemWidget->parent();
    if (!folderBloc) {
        qCDebug(lcDrivePreferencesWidget) << "Bad function sender!";
        Q_ASSERT(false);
        return nullptr;
    }

    FolderTreeItemWidget *folderTreeItemWidget = folderBloc->findChild<FolderTreeItemWidget *>();
    if (!folderTreeItemWidget) {
        qCDebug(lcDrivePreferencesWidget) << "Bad folder bloc!";
        Q_ASSERT(false);
        return nullptr;
    }

    return folderTreeItemWidget;
}

FolderItemWidget *DrivePreferencesWidget::folderItemWidget(QObject *folderTreeItemWidget)
{
    PreferencesBlocWidget *folderBloc = (PreferencesBlocWidget *) folderTreeItemWidget->parent();
    if (!folderBloc) {
        qCDebug(lcDrivePreferencesWidget) << "Bad function sender!";
        Q_ASSERT(false);
        return nullptr;
    }

    FolderItemWidget *folderItemWidget = folderBloc->findChild<FolderItemWidget *>();
    if (!folderItemWidget) {
        qCDebug(lcDrivePreferencesWidget) << "Bad folder bloc!";
        Q_ASSERT(false);
        return nullptr;
    }

    return folderItemWidget;
}

void DrivePreferencesWidget::onDisplaySmartSyncInfo(const QString &link)
{
    Q_UNUSED(link)

    // TODO
}

void DrivePreferencesWidget::onErrorsWidgetClicked()
{
    emit displayErrors(_accountId);
}

void DrivePreferencesWidget::onAddFolder(bool checked)
{
    Q_UNUSED(checked)

    QString localFolderPath = QString();
    QString serverFolderPath = QString();
    QString serverFolderBasePath = QString();
    bool serverFolderHasSubfolders = false;
    QStringList blackList = QStringList();
    QString localFolderName = QString();
    qint64 localFolderSize = 0;
    QString serverFolderName = QString();
    qint64 serverFolderSize = 0;
    AddFolderStep nextStep = SelectLocalFolder;

    while (true) {
        if (nextStep == SelectLocalFolder) {
            LocalFolderDialog *localFolderDialog = new LocalFolderDialog(localFolderPath, this);
            connect(localFolderDialog, &LocalFolderDialog::openFolder, this, &DrivePreferencesWidget::onOpenFolder);
            if (localFolderDialog->exec() == QDialog::Rejected) {
                break;
            }

            localFolderPath = localFolderDialog->localFolderPath();
            QFileInfo localFolderInfo(localFolderPath);
            localFolderName = localFolderInfo.baseName();
            localFolderSize = OCC::Utility::dirSize(localFolderPath);
            nextStep = SelectServerBaseFolder;
        }

        if (nextStep == SelectServerBaseFolder) {
            ServerBaseFolderDialog *serverBaseFolderDialog = new ServerBaseFolderDialog(_accountId, localFolderName, this);
            int ret = serverBaseFolderDialog->exec();
            if (ret == QDialog::Rejected) {
                break;
            }
            else if (ret == -1) {
                // Go back
                nextStep = SelectLocalFolder;
                continue;
            }

            serverFolderPath = serverBaseFolderDialog->serverFolderPath();
            serverFolderBasePath = serverBaseFolderDialog->serverFolderBasePath();
            QDir serverFolderDir(serverFolderPath);
            serverFolderName = serverFolderDir.dirName();
            nextStep = SelectServerFolders;
        }

        if (nextStep == SelectServerFolders) {
            serverFolderHasSubfolders = folderHasSubfolders(serverFolderPath);
            if (serverFolderHasSubfolders) {
                ServerFoldersDialog *serverFoldersDialog = new ServerFoldersDialog(_accountId, serverFolderPath, this);
                int ret = serverFoldersDialog->exec();
                if (ret == QDialog::Rejected) {
                    break;
                }
                else if (ret == -1) {
                    // Go back
                    nextStep = SelectServerBaseFolder;
                    continue;
                }

                serverFolderSize = serverFoldersDialog->selectionSize();
                blackList = serverFoldersDialog->createBlackList();
            }
            nextStep = Confirm;
        }

        if (nextStep == Confirm) {
            ConfirmSynchronizationDialog *confirmSynchronizationDialog = new ConfirmSynchronizationDialog(
                        localFolderName, localFolderSize, serverFolderName, serverFolderSize, this);
            int ret = confirmSynchronizationDialog->exec();
            if (ret == QDialog::Rejected) {
                break;
            }
            else if (ret == -1) {
                // Go back
                nextStep = serverFolderHasSubfolders ? SelectServerFolders : SelectServerBaseFolder;
                continue;
            }

            if (serverFolderPath != serverFolderBasePath) {
                // Create server folders



                if (!createFolder(serverFolderPath)) {
                    CustomMessageBox *msgBox = new CustomMessageBox(
                                QMessageBox::Warning,
                                tr("Synchronization of a new folder failed!"),
                                QMessageBox::Ok, this);
                    msgBox->setDefaultButton(QMessageBox::Ok);
                    msgBox->exec();

                    break;
                }
            }

            QStringList warnStrings;
            if (!serverFolderPath.startsWith(QLatin1Char('/'))) {
                serverFolderPath.prepend(QLatin1Char('/'));
            }

            OCC::AccountPtr accountPtr = OCC::AccountManager::instance()->getAccountFromId(_accountId);
            OCC::Folder::Map map = OCC::FolderMan::instance()->map();
            OCC::Folder::Map::const_iterator i = map.constBegin();
            for (i = map.constBegin(); i != map.constEnd(); i++) {
                OCC::Folder *folder = static_cast<OCC::Folder *>(i.value());
                if (folder->accountState()->account() != accountPtr) {
                    continue;
                }
                QString curDir = folder->remotePathTrailingSlash();
                if (QDir::cleanPath(serverFolderPath) == QDir::cleanPath(curDir))
                {
                    warnStrings.append(tr("This folder is already being synced."));
                }
                else if (serverFolderPath.startsWith(curDir))
                {
                    warnStrings.append(tr("You are already syncing <i>%1</i>, which is a parent folder of <i>%2</i>.")
                                       .arg(OCC::Utility::escape(curDir), OCC::Utility::escape(serverFolderPath)));
                }
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
            _smartSyncSwitch->setToolTip(tr("Smart synchronization activation in progress"));
            for (auto folderInfoElt : _accountInfo->_folderMap) {
                OCC::Folder *folder = OCC::FolderMan::instance()->folder(folderInfoElt.first);
                if (folder) {
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
        askDisableSmartSync([this](bool enable) {
            if (!enable) {
                _smartSyncSwitch->setCheckState(Qt::Checked);
                return;
            }

            _smartSyncSwitch->setEnabled(false);
            _smartSyncSwitch->setToolTip(tr("Smart synchronization deactivation in progress"));
            for (auto folderInfoElt : _accountInfo->_folderMap) {
                OCC::Folder *folder = OCC::FolderMan::instance()->folder(folderInfoElt.first);
                if (folder) {
                    // It is unsafe to switch off vfs while a sync is running - wait if necessary.
                    auto connection = std::make_shared<QMetaObject::Connection>();
                    if (folder->isSyncRunning()) {
                        folder->setVfsOnOffSwitchPending(true);
                        *connection = connect(folder, &OCC::Folder::syncFinished,
                                              this, [=](){ switchVfsOff(folder, connection); });
                        folder->slotTerminateSync();
                    } else {
                        switchVfsOff(folder, connection);
                    }
                }
            }
        });
    }
}

void DrivePreferencesWidget::onNotificationsSwitchClicked(bool checked)
{
    OCC::FolderMan::instance()->setNotificationsDisabled(_accountId, checked);
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
    msgBox->addButton(tr("Remove Folder Sync Connection"), QMessageBox::Yes);
    msgBox->addButton(tr("Cancel"), QMessageBox::No);
    msgBox->setDefaultButton(QMessageBox::No);
    int ret = msgBox->exec();
    if (ret != QDialog::Rejected) {
        if (ret == QMessageBox::YesRole) {
            folderMan->removeFolder(folder);
        }
    }
}

void DrivePreferencesWidget::onDisplayFolderDetail(const QString &folderId, bool display)
{
    if (_accountInfo) {
        QList<PreferencesBlocWidget *> folderBlocList = findChildren<PreferencesBlocWidget *>(folderBlocName);
        for (PreferencesBlocWidget *folderBloc : folderBlocList) {
            FolderItemWidget *folderItemWidget = folderBloc->findChild<FolderItemWidget *>();
            if (!folderItemWidget) {
                qCDebug(lcDrivePreferencesWidget) << "Bad folder bloc!";
                Q_ASSERT(false);
                break;
            }

            if (folderItemWidget->folderId() == folderId) {
                FolderTreeItemWidget *folderTreeItemWidget = folderBloc->findChild<FolderTreeItemWidget *>();
                QFrame *separatorFrame = folderBloc->findChild<QFrame *>();
                if (!folderTreeItemWidget || !separatorFrame) {
                    qCDebug(lcDrivePreferencesWidget) << "Bad folder bloc!";
                    Q_ASSERT(false);
                    break;
                }

                folderTreeItemWidget->loadSubFolders();
                folderTreeItemWidget->setVisible(display);
                separatorFrame->setVisible(display);
                break;
            }
        }
    }
}

void DrivePreferencesWidget::onOpenFolder(const QString &filePath)
{
    emit openFolder(filePath);
}

void DrivePreferencesWidget::onDisplayMessage(const QString &text)
{
    CustomMessageBox *msgBox = new CustomMessageBox(
                QMessageBox::Warning,
                text,
                QMessageBox::Ok, this);
    msgBox->setDefaultButton(QMessageBox::Ok);
    msgBox->exec();
}

void DrivePreferencesWidget::onNeedToSave()
{
    // Show update widget
    FolderItemWidget *itemWidget = folderItemWidget(sender());
    if (itemWidget) {
        itemWidget->setUpdateWidgetVisible(true);
    }
}

void DrivePreferencesWidget::onCancelUpdate()
{
    FolderTreeItemWidget *treeItemWidget = folderTreeItemWidget(sender());
    if (treeItemWidget) {
        treeItemWidget->loadSubFolders();

        // Hide update widget
        FolderItemWidget *itemWidget = (FolderItemWidget *) sender();
        if (itemWidget) {
            itemWidget->setUpdateWidgetVisible(false);
        }
    }
}

void DrivePreferencesWidget::onValidateUpdate()
{
    FolderTreeItemWidget *treeItemWidget = folderTreeItemWidget(sender());
    if (treeItemWidget) {
        OCC::Folder *folder = OCC::FolderMan::instance()->folder(treeItemWidget->folderId());
        if (folder) {
            bool ok;
            auto oldBlackListSet = folder->journalDb()->getSelectiveSyncList(OCC::SyncJournalDb::SelectiveSyncBlackList, &ok).toSet();
            if (!ok) {
                return;
            }

            QStringList blackList = treeItemWidget->createBlackList();
            folder->journalDb()->setSelectiveSyncList(OCC::SyncJournalDb::SelectiveSyncBlackList, blackList);

            if (folder->isBusy()) {
                folder->slotTerminateSync();
            }

            // The part that changed should not be read from the DB on next sync because there might be new folders
            // (the ones that are no longer in the blacklist)
            auto blackListSet = blackList.toSet();
            auto changes = (oldBlackListSet - blackListSet) + (blackListSet - oldBlackListSet);
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

}
