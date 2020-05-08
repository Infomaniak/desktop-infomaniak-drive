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
#include "preferencesblocwidget.h"
#include "errorswidget.h"
#include "accountmanager.h"
#include "configfile.h"
#include "guiutility.h"
#include "theme.h"
#include "common/utility.h"
#include "common/vfs.h"

#include <QDir>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>
#include <QStandardPaths>
#include <QStyle>

namespace KDC {

static const int boxHMargin= 20;
static const int boxVMargin = 20;
static const int boxHSpacing = 15;
static const int boxVSpacing = 12;
static const int textHSpacing = 5;
static const int progressBarMin = 0;
static const int progressBarMax = 100;
static const int dirSepIconSize = 10;
static const int dirIconSize = 18;
static const int avatarSize = 40;
static const int locationWidgetMaxSize = 450;

Q_LOGGING_CATEGORY(lcDrivePreferencesWidget, "drivepreferenceswidget", QtInfoMsg)

DrivePreferencesWidget::DrivePreferencesWidget(QWidget *parent)
    : QWidget(parent)
    , _accountId(QString())
    , _accountInfo(nullptr)
    , _progressBar(nullptr)
    , _progressLabel(nullptr)
    , _smartSyncCheckBox(nullptr)
    , _smartSyncDescriptionLabel(nullptr)
    , _locationBox(nullptr)
    , _accountAvatarLabel(nullptr)
    , _accountNameLabel(nullptr)
    , _accountMailLabel(nullptr)
    , _notificationsCheckBox(nullptr)
{
    setContentsMargins(0, 0, 0, 0);

    OCC::ConfigFile cfg;

    QVBoxLayout *vbox = new QVBoxLayout();
    vbox->setContentsMargins(boxHMargin, boxVMargin, boxHMargin, boxVMargin);
    vbox->setSpacing(boxVSpacing);
    setLayout(vbox);

    //
    // Storage bloc
    //
    QLabel *storageLabel = new QLabel(tr("Storage space"), this);
    storageLabel->setObjectName("blocLabel");
    vbox->addWidget(storageLabel);

    PreferencesBlocWidget *storageBloc = new PreferencesBlocWidget(this);
    vbox->addWidget(storageBloc);

    QBoxLayout *storageBox = storageBloc->addLayout(QBoxLayout::Direction::LeftToRight);
    storageBox->setSpacing(boxHSpacing);

    _progressBar = new QProgressBar(this);
    _progressBar->setMinimum(progressBarMin);
    _progressBar->setMaximum(progressBarMax);
    _progressBar->setFormat(QString());
    storageBox->addWidget(_progressBar);

    _progressLabel = new QLabel(this);
    _progressLabel->setObjectName("progressLabel");
    storageBox->addWidget(_progressLabel);

    //
    // Synchronization errors
    //
    ErrorsWidget *errorsWidget = new ErrorsWidget(this);
    vbox->addWidget(errorsWidget);

    //
    // Synchronization bloc
    //
    QLabel *synchronizationLabel = new QLabel(tr("Synchronization"), this);
    synchronizationLabel->setObjectName("blocLabel");
    vbox->addWidget(synchronizationLabel);

    PreferencesBlocWidget *synchronizationBloc = new PreferencesBlocWidget(this);
    vbox->addWidget(synchronizationBloc);

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

        _smartSyncCheckBox = new CustomCheckBox(this);
        _smartSyncCheckBox->setLayoutDirection(Qt::RightToLeft);
        _smartSyncCheckBox->setAttribute(Qt::WA_MacShowFocusRect, false);
        smartSync1HBox->addWidget(_smartSyncCheckBox);

        _smartSyncDescriptionLabel = new QLabel(this);
        _smartSyncDescriptionLabel->setObjectName("description");
        _smartSyncDescriptionLabel->setText(tr("Synchronize all your files without using your computer space."
                                              " <a style=\"%1\" href=\"ref\">Learn more</a>")
                                           .arg(OCC::Utility::linkStyle));
        _smartSyncDescriptionLabel->setWordWrap(true);
        smartSyncBox->addWidget(_smartSyncDescriptionLabel);
        synchronizationBloc->addSeparator();
    }

    // kDrive folders
    QVBoxLayout *driveFoldersVBox = nullptr;
    ClickableWidget *driveFoldersWidget = synchronizationBloc->addActionWidget(&driveFoldersVBox);

    QLabel *driveFoldersLabel = new QLabel(tr("kDrive folders"), this);
    driveFoldersVBox->addWidget(driveFoldersLabel);

    QLabel *driveFoldersStatusLabel = new QLabel(tr("All the folders are synchronized"), this);
    driveFoldersStatusLabel->setObjectName("description");
    driveFoldersVBox->addWidget(driveFoldersStatusLabel);

    synchronizationBloc->addSeparator();

    // Local folders
    QVBoxLayout *localFoldersVBox = nullptr;
    ClickableWidget *localFoldersWidget = synchronizationBloc->addActionWidget(&localFoldersVBox);

    QLabel *localFoldersLabel = new QLabel(tr("Folders on my computer"), this);
    localFoldersVBox->addWidget(localFoldersLabel);

    QLabel *localFoldersStatusLabel = new QLabel(tr("0 folder synchronized on the kDrive"), this);
    localFoldersStatusLabel->setObjectName("description");
    localFoldersVBox->addWidget(localFoldersStatusLabel);

    synchronizationBloc->addSeparator();

    // Other devices
    QVBoxLayout *otherDevicesVBox = nullptr;
    ClickableWidget *otherDevicesWidget = synchronizationBloc->addActionWidget(&otherDevicesVBox);

    QLabel *otherDevicesLabel = new QLabel(tr("Devices (SD card, camera, etc.)"), this);
    otherDevicesVBox->addWidget(otherDevicesLabel);

    QLabel *otherDevicesStatusLabel = new QLabel(tr("0 device configured"), this);
    otherDevicesStatusLabel->setObjectName("description");
    otherDevicesVBox->addWidget(otherDevicesStatusLabel);

    //
    // Location bloc
    //
    QLabel *locationLabel = new QLabel(tr("kDrive location"), this);
    locationLabel->setObjectName("blocLabel");
    vbox->addWidget(locationLabel);

    PreferencesBlocWidget *locationBloc = new PreferencesBlocWidget(this);
    vbox->addWidget(locationBloc);

    _locationBox = locationBloc->addLayout(QBoxLayout::Direction::LeftToRight);

    //
    // Connected with bloc
    //
    QLabel *connectedWithLabel = new QLabel(tr("Connected with"), this);
    connectedWithLabel->setObjectName("blocLabel");
    vbox->addWidget(connectedWithLabel);

    PreferencesBlocWidget *connectedWithBloc = new PreferencesBlocWidget(this);
    vbox->addWidget(connectedWithBloc);

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

    //
    // Notifications bloc
    //
    QLabel *notificationsLabel = new QLabel(tr("Notifications"), this);
    notificationsLabel->setObjectName("blocLabel");
    vbox->addWidget(notificationsLabel);

    PreferencesBlocWidget *notificationsBloc = new PreferencesBlocWidget(this);
    vbox->addWidget(notificationsBloc);

    QBoxLayout *notificationsBox = notificationsBloc->addLayout(QBoxLayout::Direction::TopToBottom);

    QHBoxLayout *notifications1HBox = new QHBoxLayout();
    notifications1HBox->setContentsMargins(0, 0, 0, 0);
    notifications1HBox->setSpacing(0);
    notificationsBox->addLayout(notifications1HBox);

    QLabel *notificationsTitleLabel = new QLabel(tr("Disable the notifications"), this);
    notifications1HBox->addWidget(notificationsTitleLabel);

    _notificationsCheckBox = new CustomCheckBox(this);
    _notificationsCheckBox->setLayoutDirection(Qt::RightToLeft);
    _notificationsCheckBox->setAttribute(Qt::WA_MacShowFocusRect, false);
    _notificationsCheckBox->setCheckState(cfg.monoIcons() ? Qt::Checked : Qt::Unchecked);
    notifications1HBox->addWidget(_notificationsCheckBox);

    QHBoxLayout *notifications2HBox = new QHBoxLayout();
    notifications2HBox->setContentsMargins(0, 0, 0, 0);
    notifications2HBox->setSpacing(0);
    notificationsBox->addLayout(notifications2HBox);

    QLabel *notificationsDescriptionLabel = new QLabel(tr("A notification will be displayed as soon as a new folder "
                                                          "has been synchronized or modified"), this);
    notificationsDescriptionLabel->setObjectName("description");
    notifications2HBox->addWidget(notificationsDescriptionLabel);

    vbox->addStretch();

    if (_smartSyncCheckBox && _smartSyncDescriptionLabel) {
        connect(_smartSyncCheckBox, &CustomCheckBox::clicked, this, &DrivePreferencesWidget::onSmartSyncCheckBoxClicked);
        connect(_smartSyncDescriptionLabel, &QLabel::linkActivated, this, &DrivePreferencesWidget::onDisplaySmartSyncInfo);
    }
    connect(driveFoldersWidget, &ClickableWidget::clicked, this, &DrivePreferencesWidget::onDriveFoldersWidgetClicked);
    connect(localFoldersWidget, &ClickableWidget::clicked, this, &DrivePreferencesWidget::onLocalFoldersWidgetClicked);
    connect(otherDevicesWidget, &ClickableWidget::clicked, this, &DrivePreferencesWidget::onOtherDevicesWidgetClicked);
    connect(_notificationsCheckBox, &CustomCheckBox::clicked, this, &DrivePreferencesWidget::onNotificationsCheckBoxClicked);
}

void DrivePreferencesWidget::setAccount(const std::map<QString, AccountInfo>::iterator accountInfoIt)
{
    _accountId = accountInfoIt->first;
    _accountInfo = &accountInfoIt->second;
    setUsedSize(_accountInfo->_totalSize, _accountInfo->_used);
    updateSmartSyncCheckBoxState();
    updateDriveLocation();
    updateAccountInfo();
    _notificationsCheckBox->setChecked(OCC::FolderMan::instance()->notificationsDisabled(_accountId));
}

void DrivePreferencesWidget::reset()
{
    _accountId = QString();
    _accountInfo = nullptr;
    setUsedSize(0, 0);
    _smartSyncCheckBox->setChecked(false);
    resetDriveLocation();
    _accountAvatarLabel->setPixmap(QPixmap());
    _accountNameLabel->setText(QString());
    _accountMailLabel->setText(QString());
    _notificationsCheckBox->setChecked(false);
}

void DrivePreferencesWidget::setUsedSize(qint64 totalSize, qint64 size)
{
    _progressBar->setVisible(true);
    _progressLabel->setVisible(true);
    if (totalSize > 0) {
        int pct = size <= totalSize ? qRound(double(size) / double(totalSize) * 100.0) : 100;
        _progressBar->setValue(pct);
        _progressLabel->setText(OCC::Utility::octetsToString(size) + " / " + OCC::Utility::octetsToString(totalSize));
    }
    else {
        // -1 => not computed; -2 => unknown; -3 => unlimited
        if (totalSize == 0 || totalSize == -1) {
            _progressBar->setValue(0);
            _progressLabel->setText(QString());
        } else {
            _progressBar->setValue(0);
            _progressLabel->setText(tr("%1 in use").arg(OCC::Utility::octetsToString(size)));
        }
    }
}

void DrivePreferencesWidget::updateSmartSyncCheckBoxState()
{
    if (_smartSyncCheckBox) {
        bool smartSyncAvailable = false;
        bool oneDoesntSupportsVirtualFiles = false;
        bool oneHasVfsOnOffSwitchPending = false;
        for (auto folderInfoIt : _accountInfo->_folderMap) {
            OCC::Folder *folder = OCC::FolderMan::instance()->folder(folderInfoIt.first);
            if (folder) {
                oneDoesntSupportsVirtualFiles |= !folder->supportsVirtualFiles();
                oneHasVfsOnOffSwitchPending |= folder->isVfsOnOffSwitchPending();
            }
        }
        smartSyncAvailable = oneDoesntSupportsVirtualFiles && !oneHasVfsOnOffSwitchPending;
        _smartSyncCheckBox->setCheckState(smartSyncAvailable ? Qt::Unchecked : Qt::Checked);
        _smartSyncCheckBox->setEnabled(!oneHasVfsOnOffSwitchPending);
        if (!oneHasVfsOnOffSwitchPending) {
            _smartSyncCheckBox->setToolTip("");
        }
    }
}

void DrivePreferencesWidget::resetDriveLocation()
{
    QWidget *locationWidget = findChild<QWidget *>("locationWidget");
    if (locationWidget) {
        delete locationWidget;
    }
}

void DrivePreferencesWidget::updateDriveLocation()
{
    resetDriveLocation();
    QWidget *locationWidget = new QWidget(this);
    locationWidget->setObjectName("locationWidget");
    QHBoxLayout *hBox = new QHBoxLayout();
    hBox->setContentsMargins(0, 0, 0, 0);
    locationWidget->setLayout(hBox);
    _locationBox->addWidget(locationWidget);

    auto folderInfoIt = _accountInfo->_folderMap.begin();
    if (folderInfoIt != _accountInfo->_folderMap.end()) {
        if (folderInfoIt->second) {
            QString folderPath = QDir(folderInfoIt->second->_path).absolutePath();
            QString homePath = QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).absolutePath();
            QStringList folderPathPartList;
            QString homeDir = QString();
            if (folderPath.left(homePath.size()) == homePath) {
                // Drive folder is a subdirectory of Home directory
                homeDir = QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).dirName();

                QString folderPathFromHome = folderPath.right(folderPath.size() - homePath.size());
                folderPathPartList = folderPathFromHome.split("/");
            }
            else {
                folderPathPartList = folderPath.split("/");
            }

            if (homeDir.isEmpty()) {
                QLabel *dirIconLabel = new QLabel(this);
                QIcon computerIcon = style()->standardIcon(QStyle::SP_DriveHDIcon);
                dirIconLabel->setPixmap(computerIcon.pixmap(dirIconSize, dirIconSize));
                hBox->addWidget(dirIconLabel);
            }
            else {
                QLabel *dirIconLabel = new QLabel(this);
                QIcon homeIcon = style()->standardIcon(QStyle::SP_DirHomeIcon);
                dirIconLabel->setPixmap(homeIcon.pixmap(dirIconSize, dirIconSize));
                hBox->addWidget(dirIconLabel);

                QLabel *dirLabel = new QLabel(homeDir, this);
                hBox->addWidget(dirLabel);
            }

            QIcon dirIcon = style()->standardIcon(QStyle::SP_DirIcon);
            for (QString folderPathPart : folderPathPartList) {
                if (!folderPathPart.isEmpty() && locationWidget->sizeHint().width() < locationWidgetMaxSize) {
                    QLabel *dirSepIconLabel = new QLabel(this);
                    dirSepIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/chevron-right.svg")
                                               .pixmap(dirSepIconSize, dirSepIconSize));
                    hBox->addWidget(dirSepIconLabel);

                    QLabel *dirIconLabel = new QLabel(this);
                    dirIconLabel->setPixmap(dirIcon.pixmap(dirIconSize, dirIconSize));
                    hBox->addWidget(dirIconLabel);

                    QLabel *dirLabel = new QLabel(folderPathPart, this);
                    hBox->addWidget(dirLabel);
                }
            }
            hBox->addStretch();
        }
        else {
            qCDebug(lcDrivePreferencesWidget) << "Null pointer!";
            Q_ASSERT(false);
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
    QMessageBox *msgBox = nullptr;
    if (bestVfsMode == OCC::Vfs::WindowsCfApi) {
        msgBox = new QMessageBox(
            QMessageBox::Warning,
            tr("Enable technical preview feature?"),
            tr("When the \"virtual files\" mode is enabled no files will be downloaded initially. "
               "Instead a virtual file will be created for each file that exists on the server. "
               "When a file is opened its contents will be downloaded automatically. "
               "Alternatively, files can be downloaded manually by using their context menu.\n\n"
               "The virtual files mode is mutually exclusive with selective sync. "
               "Currently unselected folders will be translated to online-only folders "
               "and your selective sync settings will be reset."));
        msgBox->addButton(tr("Enable virtual files"), QMessageBox::AcceptRole);
        msgBox->addButton(tr("Continue to use selective sync"), QMessageBox::RejectRole);
    } else {
        ASSERT(bestVfsMode == OCC::Vfs::WithSuffix)
        msgBox = new QMessageBox(
            QMessageBox::Warning,
            tr("Enable experimental feature?"),
            tr("When the \"virtual files\" mode is enabled no files will be downloaded initially. "
               "Instead, a tiny \"%1\" file will be created for each file that exists on the server. "
               "The contents can be downloaded by running these files or by using their context menu.\n\n"
               "The virtual files mode is mutually exclusive with selective sync. "
               "Currently unselected folders will be translated to online-only folders "
               "and your selective sync settings will be reset.\n\n"
               "Switching to this mode will abort any currently running synchronization.\n\n"
               "This is a new, experimental mode. If you decide to use it, please report any "
               "issues that come up.")
                .arg(APPLICATION_DOTVIRTUALFILE_SUFFIX));
        msgBox->addButton(tr("Enable experimental placeholder mode"), QMessageBox::AcceptRole);
        msgBox->addButton(tr("Stay safe"), QMessageBox::RejectRole);
    }
    connect(msgBox, &QMessageBox::finished, msgBox, [callback, msgBox](int result) {
        callback(result == QMessageBox::AcceptRole);
        msgBox->deleteLater();
    });
    msgBox->open();
}

void DrivePreferencesWidget::askDisableSmartSync(const std::function<void (bool)> &callback)
{
    auto msgBox = new QMessageBox(
        QMessageBox::Question,
        tr("Disable virtual file support?"),
        tr("This action will disable virtual file support. As a consequence contents of folders that "
           "are currently marked as 'available online only' will be downloaded.\n\n"
           "This action will abort any currently running synchronization."));
    msgBox->addButton(tr("Disable support"), QMessageBox::AcceptRole);
    msgBox->addButton(tr("Cancel"), QMessageBox::RejectRole);
    connect(msgBox, &QMessageBox::finished, msgBox, [callback, msgBox](int result) {
        callback(result == QMessageBox::AcceptRole);
        msgBox->deleteLater();
    });
    msgBox->open();
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

    updateSmartSyncCheckBoxState();
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

    updateSmartSyncCheckBoxState();
}

void DrivePreferencesWidget::onDisplaySmartSyncInfo(const QString &link)
{
    Q_UNUSED(link)


}

void DrivePreferencesWidget::onSmartSyncCheckBoxClicked(bool checked)
{
    if (checked) {
        askEnableSmartSync([this](bool enable) {
            if (!enable) {
                _smartSyncCheckBox->setCheckState(Qt::Unchecked);
                return;
            }

            _smartSyncCheckBox->setEnabled(false);
            _smartSyncCheckBox->setToolTip(tr("Smart synchronization activation in progress"));
            for (auto folderInfoIt : _accountInfo->_folderMap) {
                OCC::Folder *folder = OCC::FolderMan::instance()->folder(folderInfoIt.first);
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
                _smartSyncCheckBox->setCheckState(Qt::Checked);
                return;
            }

            _smartSyncCheckBox->setEnabled(false);
            _smartSyncCheckBox->setToolTip(tr("Smart synchronization deactivation in progress"));
            for (auto folderInfoIt : _accountInfo->_folderMap) {
                OCC::Folder *folder = OCC::FolderMan::instance()->folder(folderInfoIt.first);
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

void DrivePreferencesWidget::onDriveFoldersWidgetClicked()
{

}

void DrivePreferencesWidget::onLocalFoldersWidgetClicked()
{

}

void DrivePreferencesWidget::onOtherDevicesWidgetClicked()
{

}

void DrivePreferencesWidget::onNotificationsCheckBoxClicked(bool checked)
{
    OCC::FolderMan::instance()->setNotificationsDisabled(_accountId, checked);
}

}
