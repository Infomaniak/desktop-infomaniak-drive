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
#include "serverfoldersdialog.h"
#include "custommessagebox.h"
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
    QLabel *foldersLabel = new QLabel(tr("Folders"), this);
    foldersLabel->setObjectName("blocLabel");
    _mainVBox->addWidget(foldersLabel);
    _foldersBeginIndex = _mainVBox->indexOf(foldersLabel) + 1;

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

    _mainVBox->addStretch();

    connect(_displayErrorsWidget, &ActionWidget::clicked, this, &DrivePreferencesWidget::onErrorsWidgetClicked);
    if (_smartSyncSwitch && _smartSyncDescriptionLabel) {
        connect(_smartSyncSwitch, &CustomSwitch::clicked, this, &DrivePreferencesWidget::onSmartSyncSwitchClicked);
        connect(_smartSyncDescriptionLabel, &QLabel::linkActivated, this, &DrivePreferencesWidget::onDisplaySmartSyncInfo);
    }
    connect(_notificationsSwitch, &CustomSwitch::clicked, this, &DrivePreferencesWidget::onNotificationsSwitchClicked);
    connect(this, &DrivePreferencesWidget::errorAdded, this, &DrivePreferencesWidget::onErrorAdded);
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
        msgBox->addButton(tr("Enable virtual files"), QMessageBox::AcceptRole);
        msgBox->addButton(tr("Continue to use selective sync"), QMessageBox::RejectRole);
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
        msgBox->addButton(tr("Enable experimental placeholder mode"), QMessageBox::AcceptRole);
        msgBox->addButton(tr("Stay safe"), QMessageBox::RejectRole);
    }
    int result = msgBox->exec();
    callback(result == QMessageBox::AcceptRole);
}

void DrivePreferencesWidget::askDisableSmartSync(const std::function<void (bool)> &callback)
{
    CustomMessageBox *msgBox = new CustomMessageBox(
                QMessageBox::Question,
                tr("This action will disable virtual file support. As a consequence contents of folders that "
                   "are currently marked as 'available online only' will be downloaded.\n\n"
                   "This action will abort any currently running synchronization."),
                QMessageBox::NoButton, this);
    msgBox->addButton(tr("Disable support"), QMessageBox::AcceptRole);
    msgBox->addButton(tr("Cancel"), QMessageBox::RejectRole);
    int result = msgBox->exec();
    callback(result == QMessageBox::AcceptRole);
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
                QBoxLayout *folderTreeBox = folderBloc->addLayout(QBoxLayout::Direction::LeftToRight);

                FolderTreeItemWidget *folderTreeItemWidget = new FolderTreeItemWidget(folderInfoElt.first, true, this);
                folderTreeItemWidget->setVisible(false);
                folderTreeBox->addWidget(folderTreeItemWidget);

                connect(folderItemWidget, &FolderItemWidget::runSync, this, &DrivePreferencesWidget::onSyncTriggered);
                connect(folderItemWidget, &FolderItemWidget::pauseSync, this, &DrivePreferencesWidget::onPauseTriggered);
                connect(folderItemWidget, &FolderItemWidget::resumeSync, this, &DrivePreferencesWidget::onResumeTriggered);
                connect(folderItemWidget, &FolderItemWidget::unSync, this, &DrivePreferencesWidget::onUnsyncTriggered);
                connect(folderItemWidget, &FolderItemWidget::displayFolderDetail, this, &DrivePreferencesWidget::onDisplayFolderDetail);
            }
        }

        update();
    }
}

void DrivePreferencesWidget::onDisplaySmartSyncInfo(const QString &link)
{
    Q_UNUSED(link)


}

void DrivePreferencesWidget::onErrorsWidgetClicked()
{
    emit displayErrors(_accountId);
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

void DrivePreferencesWidget::onDriveFoldersWidgetClicked()
{
    ServerFoldersDialog *dialog = new ServerFoldersDialog(_accountInfo, this);
    dialog->exec();
}

void DrivePreferencesWidget::onNotificationsSwitchClicked(bool checked)
{
    OCC::FolderMan::instance()->setNotificationsDisabled(_accountId, checked);
}

void DrivePreferencesWidget::onErrorAdded()
{
    _displayErrorsWidget->setVisible(true);
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

}

void DrivePreferencesWidget::onDisplayFolderDetail(const QString &folderId, bool display)
{
    if (_accountInfo) {
        QList<PreferencesBlocWidget *> folderBlocList = findChildren<PreferencesBlocWidget *>(folderBlocName);
        for (PreferencesBlocWidget *folderBloc : folderBlocList) {
            FolderItemWidget *folderItemWidget = folderBloc->findChild<FolderItemWidget *>();
            if (folderItemWidget) {
                if (folderItemWidget->folderId() == folderId) {
                    FolderTreeItemWidget *folderTreeItemWidget = folderBloc->findChild<FolderTreeItemWidget *>();
                    folderTreeItemWidget->loadSubFolders();
                    folderTreeItemWidget->setVisible(display);
                    QFrame *separatorFrame = folderBloc->findChild<QFrame *>();
                    separatorFrame->setVisible(display);
                    break;
                }
            }
            else {
                qCDebug(lcDrivePreferencesWidget) << "Empty folder bloc!";
                Q_ASSERT(false);
            }
        }
    }
}

}
