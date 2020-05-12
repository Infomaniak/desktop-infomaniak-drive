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

#include "synthesispopover.h"
#include "menuitemwidget.h"
#include "menuwidget.h"
#include "bottomwidget.h"
#include "custompushbutton.h"
#include "synchronizeditem.h"
#include "accountmanager.h"
#include "folderman.h"
#include "account.h"
#include "common/utility.h"
#include "guiutility.h"
#include "openfilemanager.h"
#include "getorcreatepubliclinkshare.h"
#include "configfile.h"
#include "theme.h"

#define CONSOLE_DEBUG
#ifdef CONSOLE_DEBUG
#include <iostream>
#endif

#include <QActionGroup>
#include <QApplication>
#include <QBoxLayout>
#include <QClipboard>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QWidgetAction>

namespace KDC {

static const int triangleHeight = 10;
static const int triangleWidth  = 20;
static const int trianglePosition = 100; // Position from side
static const int cornerRadius = 5;
static const int shadowBlurRadius = 20;
static const int toolBarHMargin = 10;
static const int toolBarVMargin = 10;
static const int toolBarSpacing = 10;
static const int driveBarHMargin = 10;
static const int driveBarVMargin = 10;
static const int driveBarSpacing = 15;
static const int logoIconSize = 30;
static const int defaultLogoIconSize = 50;
static const int maxSynchronizedItems = 1000;

const std::map<SynthesisPopover::NotificationsDisabled, QString> SynthesisPopover::_notificationsDisabledMap = {
    { NotificationsDisabled::Never, QString(tr("Never")) },
    { NotificationsDisabled::OneHour, QString(tr("During 1 hour")) },
    { NotificationsDisabled::UntilTomorrow, QString(tr("Until tomorrow 8:00AM")) },
    { NotificationsDisabled::TreeDays, QString(tr("During 3 days")) },
    { NotificationsDisabled::OneWeek, QString(tr("During 1 week")) },
    { NotificationsDisabled::Always, QString(tr("Always")) }
};

const std::map<SynthesisPopover::NotificationsDisabled, QString> SynthesisPopover::_notificationsDisabledForPeriodMap = {
    { NotificationsDisabled::Never, QString(tr("Never")) },
    { NotificationsDisabled::OneHour, QString(tr("For 1 more hour")) },
    { NotificationsDisabled::UntilTomorrow, QString(tr("Until tomorrow 8:00AM")) },
    { NotificationsDisabled::TreeDays, QString(tr("For 3 more days")) },
    { NotificationsDisabled::OneWeek, QString(tr("For 1 more week")) },
    { NotificationsDisabled::Always, QString(tr("Always")) }
};

Q_LOGGING_CATEGORY(lcSynthesisPopover, "synthesispopover", QtInfoMsg)

SynthesisPopover::SynthesisPopover(bool debugMode, QWidget *parent)
    : QDialog(parent)
    , _debugMode(debugMode)
    , _sysTrayIconRect(QRect())
    , _currentAccountId(QString())
    , _backgroundMainColor(QColor())
    , _folderButton(nullptr)
    , _webviewButton(nullptr)
    , _menuButton(nullptr)
    , _driveSelectionWidget(nullptr)
    , _progressBarWidget(nullptr)
    , _statusBarWidget(nullptr)
    , _buttonsBarWidget(nullptr)
    , _stackedWidget(nullptr)
    , _defaultSynchronizedPageWidget(nullptr)
    , _notificationsDisabled(NotificationsDisabled::Never)
    , _notificationsDisabledUntilDateTime(QDateTime())
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::X11BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground);

    OCC::ConfigFile cfg;
    _notificationsDisabled = cfg.optionalDesktopNotifications()
            ? NotificationsDisabled::Never
            : NotificationsDisabled::Always;

    initUI();

    connect(OCC::FolderMan::instance(), &OCC::FolderMan::folderSyncStateChange,
            this, &SynthesisPopover::onRefreshAccountList);
    connect(OCC::AccountManager::instance(), &OCC::AccountManager::accountAdded,
            this, &SynthesisPopover::onRefreshAccountList);
    connect(OCC::AccountManager::instance(), &OCC::AccountManager::accountRemoved,
            this, &SynthesisPopover::onRefreshAccountList);
    connect(OCC::ProgressDispatcher::instance(), &OCC::ProgressDispatcher::progressInfo,
            this, &SynthesisPopover::onUpdateProgress);
    connect(OCC::ProgressDispatcher::instance(), &OCC::ProgressDispatcher::itemCompleted,
            this, &SynthesisPopover::onItemCompleted);
}

void SynthesisPopover::setPosition(const QRect &sysTrayIconRect)
{
    _sysTrayIconRect = sysTrayIconRect;
}

void SynthesisPopover::changeEvent(QEvent *event)
{
    QDialog::changeEvent(event);

    switch (event->type()) {
    case QEvent::PaletteChange:
    case QEvent::ThemeChange:
        emit applyStyle();
        break;
    default:
        break;
    }
}

void SynthesisPopover::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QScreen *screen = QGuiApplication::screenAt(_sysTrayIconRect.center());
    if (!screen) {
        return;
    }

    OCC::Utility::systrayPosition position = OCC::Utility::getSystrayPosition(screen);

    // Calculate dialog position (left/top corner) && border path
    QPoint popoverPosition;
    QPainterPath painterPath;
    QRect screenRect = screen->availableGeometry();
    QRect intRect = rect().marginsRemoved(QMargins(triangleHeight, triangleHeight, triangleHeight - 1, triangleHeight - 1));
    if (_sysTrayIconRect == QRect()
            || !OCC::Utility::isPointInSystray(screen, _sysTrayIconRect.center())) {
        // Unknown Systray icon position (Linux) or icon not in Systray (Windows group)

        // Dialog position
        if (position == OCC::Utility::systrayPosition::Top) {
            popoverPosition = QPoint(
                screenRect.x() + screenRect.width() - rect().width() - triangleHeight,
                screenRect.y() + triangleHeight);
        }
        else if (position == OCC::Utility::systrayPosition::Bottom) {
            popoverPosition = QPoint(
                screenRect.x() + screenRect.width() - rect().width() - triangleHeight,
                screenRect.y() + screenRect.height() - rect().height() - triangleHeight);
        }
        else if (position == OCC::Utility::systrayPosition::Left) {
            popoverPosition = QPoint(
                screenRect.x() + triangleHeight,
                screenRect.y() + screenRect.height() - rect().height() - triangleHeight);
        }
        else if (position == OCC::Utility::systrayPosition::Right) {
            popoverPosition = QPoint(
                screenRect.x() + screenRect.width() - rect().width() - triangleHeight,
                screenRect.y() + screenRect.height() - rect().height() - triangleHeight);
        }

        // Border
        painterPath.addRoundedRect(intRect, cornerRadius, cornerRadius);
    }
    else {
        int cornerDiameter = 2 * cornerRadius;
        QPointF trianglePoint1;
        QPointF trianglePoint2;
        QPointF trianglePoint3;
        if (position == OCC::Utility::systrayPosition::Top) {
            // Triangle position (left/right)
            bool trianglePositionLeft =
                    (_sysTrayIconRect.center().x() + rect().width() - trianglePosition
                    < screenRect.x() + screenRect.width());

            // Dialog position
            popoverPosition = QPoint(
                trianglePositionLeft
                ? _sysTrayIconRect.center().x() - trianglePosition
                : _sysTrayIconRect.center().x() - rect().width() + trianglePosition,
                _sysTrayIconRect.bottom());

            // Triangle points
            trianglePoint1 = QPoint(
                trianglePositionLeft ? trianglePosition - triangleWidth / 2.0 : rect().width() - trianglePosition - triangleWidth / 2.0,
                triangleHeight);
            trianglePoint2 = QPoint(
                trianglePositionLeft ? trianglePosition : rect().width()  - trianglePosition,
                0);
            trianglePoint3 = QPoint(
                trianglePositionLeft ? trianglePosition + triangleWidth / 2.0 : rect().width() - trianglePosition + triangleWidth / 2.0,
                triangleHeight);

            // Border
            painterPath.moveTo(trianglePoint3);
            painterPath.lineTo(trianglePoint2);
            painterPath.lineTo(trianglePoint1);
            painterPath.arcTo(QRect(intRect.topLeft(), QSize(cornerDiameter, cornerDiameter)), 90, 90);
            painterPath.arcTo(QRect(intRect.bottomLeft() - QPoint(0, cornerDiameter), QSize(cornerDiameter, cornerDiameter)), 180, 90);
            painterPath.arcTo(QRect(intRect.bottomRight() - QPoint(cornerDiameter, cornerDiameter), QSize(cornerDiameter, cornerDiameter)), 270, 90);
            painterPath.arcTo(QRect(intRect.topRight() - QPoint(cornerDiameter, 0), QSize(cornerDiameter, cornerDiameter)), 0, 90);
            painterPath.closeSubpath();
        }
        else if (position == OCC::Utility::systrayPosition::Bottom) {
            // Triangle position (left/right)
            bool trianglePositionLeft =
                    (_sysTrayIconRect.center().x() + rect().width() - trianglePosition
                    < screenRect.x() + screenRect.width());

            // Dialog position
            popoverPosition = QPoint(
                trianglePositionLeft
                ? _sysTrayIconRect.center().x() - trianglePosition
                : _sysTrayIconRect.center().x() - rect().width() + trianglePosition,
                _sysTrayIconRect.top() - rect().height());

            // Triangle points
            trianglePoint1 = QPoint(
                trianglePositionLeft ? trianglePosition - triangleWidth / 2.0 : rect().width() - trianglePosition - triangleWidth / 2.0,
                rect().height() - triangleHeight);
            trianglePoint2 = QPoint(
                trianglePositionLeft ? trianglePosition : rect().width() - trianglePosition,
                rect().height());
            trianglePoint3 = QPoint(
                trianglePositionLeft ? trianglePosition + triangleWidth / 2.0 : rect().width() - trianglePosition + triangleWidth / 2.0,
                rect().height() - triangleHeight);

            // Border
            painterPath.moveTo(trianglePoint1);
            painterPath.lineTo(trianglePoint2);
            painterPath.lineTo(trianglePoint3);
            painterPath.arcTo(QRect(intRect.bottomRight() - QPoint(cornerDiameter, cornerDiameter), QSize(cornerDiameter, cornerDiameter)), 270, 90);
            painterPath.arcTo(QRect(intRect.topRight() - QPoint(cornerDiameter, 0), QSize(cornerDiameter, cornerDiameter)), 0, 90);
            painterPath.arcTo(QRect(intRect.topLeft(), QSize(cornerDiameter, cornerDiameter)), 90, 90);
            painterPath.arcTo(QRect(intRect.bottomLeft() - QPoint(0, cornerDiameter), QSize(cornerDiameter, cornerDiameter)), 180, 90);
            painterPath.closeSubpath();
        }
        else if (position == OCC::Utility::systrayPosition::Left) {
            // Triangle position (top/bottom)
            bool trianglePositionTop =
                    (_sysTrayIconRect.center().y() + rect().height() - trianglePosition
                    < screenRect.y() + screenRect.height());

            // Dialog position
            popoverPosition = QPoint(
                _sysTrayIconRect.right(),
                trianglePositionTop
                ? _sysTrayIconRect.center().y() - trianglePosition
                : _sysTrayIconRect.center().y() - rect().height() + trianglePosition);

            // Triangle points
            trianglePoint1 = QPoint(
                triangleHeight,
                trianglePositionTop ? trianglePosition - triangleWidth / 2.0 : rect().height() - trianglePosition - triangleWidth / 2.0);
            trianglePoint2 = QPoint(
                0,
                trianglePositionTop ? trianglePosition : rect().height() - trianglePosition);
            trianglePoint3 = QPoint(
                triangleHeight,
                trianglePositionTop ? trianglePosition + triangleWidth / 2.0 : rect().height() - trianglePosition + triangleWidth / 2.0);

            // Border
            painterPath.moveTo(trianglePoint1);
            painterPath.lineTo(trianglePoint2);
            painterPath.lineTo(trianglePoint3);
            painterPath.arcTo(QRect(intRect.bottomLeft() - QPoint(0, cornerDiameter), QSize(cornerDiameter, cornerDiameter)), 180, 90);
            painterPath.arcTo(QRect(intRect.bottomRight() - QPoint(cornerDiameter, cornerDiameter), QSize(cornerDiameter, cornerDiameter)), 270, 90);
            painterPath.arcTo(QRect(intRect.topRight() - QPoint(cornerDiameter, 0), QSize(cornerDiameter, cornerDiameter)), 0, 90);
            painterPath.arcTo(QRect(intRect.topLeft(), QSize(cornerDiameter, cornerDiameter)), 90, 90);
            painterPath.closeSubpath();
        }
        else if (position == OCC::Utility::systrayPosition::Right) {
            // Triangle position (top/bottom)
            bool trianglePositionTop =
                    (_sysTrayIconRect.center().y() + rect().height() - trianglePosition
                    < screenRect.y() + screenRect.height());

            // Dialog position
            popoverPosition = QPoint(
                _sysTrayIconRect.left() - rect().width(),
                trianglePositionTop
                ? _sysTrayIconRect.center().y() - trianglePosition
                : _sysTrayIconRect.center().y() - rect().height() + trianglePosition);

            // Triangle
            trianglePoint1 = QPoint(
                rect().width() - triangleHeight,
                trianglePositionTop ? trianglePosition - triangleWidth / 2.0 : rect().height() - trianglePosition - triangleWidth / 2.0);
            trianglePoint2 = QPoint(
                rect().width(),
                trianglePositionTop ? trianglePosition : rect().height() - trianglePosition);
            trianglePoint3 = QPoint(
                rect().width() - triangleHeight,
                trianglePositionTop ? trianglePosition + triangleWidth / 2.0 : rect().height() - trianglePosition + triangleWidth / 2.0);

            // Border
            painterPath.moveTo(trianglePoint3);
            painterPath.lineTo(trianglePoint2);
            painterPath.lineTo(trianglePoint1);
            painterPath.arcTo(QRect(intRect.topRight() - QPoint(cornerDiameter, 0), QSize(cornerDiameter, cornerDiameter)), 0, 90);
            painterPath.arcTo(QRect(intRect.topLeft(), QSize(cornerDiameter, cornerDiameter)), 90, 90);
            painterPath.arcTo(QRect(intRect.bottomLeft() - QPoint(0, cornerDiameter), QSize(cornerDiameter, cornerDiameter)), 180, 90);
            painterPath.arcTo(QRect(intRect.bottomRight() - QPoint(cornerDiameter, cornerDiameter), QSize(cornerDiameter, cornerDiameter)), 270, 90);
            painterPath.closeSubpath();
        }
    }

    move(popoverPosition);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(backgroundMainColor());
    painter.setPen(Qt::NoPen);
    painter.drawPath(painterPath);
}

bool SynthesisPopover::event(QEvent *event)
{
    bool ret = QWidget::event(event);
    if (event->type() == QEvent::WindowDeactivate) {
        done(QDialog::Accepted);
    }
    return ret;
}

void SynthesisPopover::initUI()
{
    /*
     *  mainVBox
     *      hBoxToolBar
     *          iconLabel
     *          _folderButton
     *          _webviewButton
     *          _menuButton
     *      hBoxDriveBar
     *          _driveSelectionWidget
     *      _progressBarWidget
     *      _statusBarWidget
     *      _buttonsBarWidget
     *          synchronizedButton
     *          favoritesButton
     *          activityButton
     *      _stackedWidget
     *          _defaultSynchronizedPageWidget
     *          notImplementedLabel
     *          notImplementedLabel2
     *          _synchronizedListWidget[]
     *      bottomWidget
     */

    QVBoxLayout *mainVBox = new QVBoxLayout();
    mainVBox->setContentsMargins(triangleHeight, triangleHeight, triangleHeight, triangleHeight);
    mainVBox->setSpacing(0);
    setLayout(mainVBox);

    // Tool bar
    QHBoxLayout *hBoxToolBar = new QHBoxLayout();
    hBoxToolBar->setContentsMargins(toolBarHMargin, toolBarVMargin, toolBarHMargin, toolBarVMargin);
    hBoxToolBar->setSpacing(toolBarSpacing);
    mainVBox->addLayout(hBoxToolBar);

    QLabel *iconLabel = new QLabel(this);
    iconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/logos/kdrive-without-text.svg")
                         .pixmap(logoIconSize, logoIconSize));
    hBoxToolBar->addWidget(iconLabel);
    hBoxToolBar->addStretch();

    _folderButton = new CustomToolButton(this);
    _folderButton->setIconPath(":/client/resources/icons/actions/folder.svg");
    _folderButton->setToolTip(tr("Show in folder"));
    hBoxToolBar->addWidget(_folderButton);

    _webviewButton = new CustomToolButton(this);
    _webviewButton->setIconPath(":/client/resources/icons/actions/webview.svg");
    _webviewButton->setToolTip(tr("Display on drive.infomaniak.com"));
    hBoxToolBar->addWidget(_webviewButton);

    _menuButton = new CustomToolButton(this);
    _menuButton->setIconPath(":/client/resources/icons/actions/menu.svg");
    _menuButton->setToolTip(tr("More actions"));
    hBoxToolBar->addWidget(_menuButton);

    // Drive selection
    QHBoxLayout *hBoxDriveBar = new QHBoxLayout();
    hBoxDriveBar->setContentsMargins(driveBarHMargin, driveBarVMargin, driveBarHMargin, driveBarVMargin);
    hBoxDriveBar->setSpacing(driveBarSpacing);

    _driveSelectionWidget = new DriveSelectionWidget(this);
    hBoxDriveBar->addWidget(_driveSelectionWidget);

    hBoxDriveBar->addStretch();
    mainVBox->addLayout(hBoxDriveBar);

    // Progress bar
    _progressBarWidget = new ProgressBarWidget(this);
    mainVBox->addWidget(_progressBarWidget);

    // Status bar
    _statusBarWidget = new StatusBarWidget(this);
    mainVBox->addWidget(_statusBarWidget);

    // Buttons bar
    _buttonsBarWidget = new ButtonsBarWidget(this);
    mainVBox->addWidget(_buttonsBarWidget);

    CustomPushButton *synchronizedButton = new CustomPushButton(tr("Synchronized"), _buttonsBarWidget);
    synchronizedButton->setIconPath(":/client/resources/icons/actions/sync.svg");
    _buttonsBarWidget->insertButton(StackedWidget::Synchronized, synchronizedButton);

    CustomPushButton *favoritesButton = new CustomPushButton(tr("Favorites"), _buttonsBarWidget);
    favoritesButton->setIconPath(":/client/resources/icons/actions/favorite.svg");
    _buttonsBarWidget->insertButton(StackedWidget::Favorites, favoritesButton);

    CustomPushButton *activityButton = new CustomPushButton(tr("Activity"), _buttonsBarWidget);
    activityButton->setIconPath(":/client/resources/icons/actions/notifications.svg");
    _buttonsBarWidget->insertButton(StackedWidget::Activity, activityButton);

    // Stacked widget
    _stackedWidget = new QStackedWidget(this);
    mainVBox->addWidget(_stackedWidget);
    mainVBox->setStretchFactor(_stackedWidget, 1);

    setSynchronizedDefaultPage(&_defaultSynchronizedPageWidget, this);
    _stackedWidget->insertWidget(StackedWidget::Synchronized, _defaultSynchronizedPageWidget);

    QLabel *notImplementedLabel = new QLabel(tr("Not implemented!"), this);
    notImplementedLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    notImplementedLabel->setObjectName("defaultTitleLabel");
    _stackedWidget->insertWidget(StackedWidget::Favorites, notImplementedLabel);

    QLabel *notImplementedLabel2 = new QLabel(tr("Not implemented!"), this);
    notImplementedLabel2->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    notImplementedLabel2->setObjectName("defaultTitleLabel");
    _stackedWidget->insertWidget(StackedWidget::Activity, notImplementedLabel2);

    // Bottom
    BottomWidget *bottomWidget = new BottomWidget(this);
    mainVBox->addWidget(bottomWidget);

    // Shadow
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
    effect->setBlurRadius(shadowBlurRadius);
    effect->setOffset(0);
    effect->setColor(OCC::Utility::getShadowColor());
    setGraphicsEffect(effect);

    connect(_folderButton, &CustomToolButton::clicked, this, &SynthesisPopover::onOpenFolderMenu);
    connect(_webviewButton, &CustomToolButton::clicked, this, &SynthesisPopover::onOpenWebview);
    connect(_menuButton, &CustomToolButton::clicked, this, &SynthesisPopover::onOpenMiscellaneousMenu);
    connect(_driveSelectionWidget, &DriveSelectionWidget::driveSelected, this, &SynthesisPopover::onAccountSelected);
    connect(_driveSelectionWidget, &DriveSelectionWidget::addDrive, this, &SynthesisPopover::onAddDrive);
    connect(_statusBarWidget, &StatusBarWidget::pauseSync, this, &SynthesisPopover::onPauseSync);
    connect(_statusBarWidget, &StatusBarWidget::resumeSync, this, &SynthesisPopover::onResumeSync);
    connect(_statusBarWidget, &StatusBarWidget::runSync, this, &SynthesisPopover::onRunSync);
    connect(_statusBarWidget, &StatusBarWidget::linkActivated, this, &SynthesisPopover::onLinkActivated);
    connect(_buttonsBarWidget, &ButtonsBarWidget::buttonToggled, this, &SynthesisPopover::onButtonBarToggled);
}

QUrl SynthesisPopover::folderUrl(const QString &folderId, const QString &filePath)
{
    QUrl url = QUrl();
    QString fullFilePath = folderPath(folderId, filePath);
    if (!fullFilePath.isEmpty()) {
#ifndef Q_OS_WIN
        url = QUrl::fromLocalFile(fullFilePath);
#else
        // work around a bug in QDesktopServices on Win32, see i-net
        if (fullFilePath.startsWith(QLatin1String("\\\\")) || fullFilePath.startsWith(QLatin1String("//"))) {
            url = QUrl::fromLocalFile(QDir::toNativeSeparators(fullFilePath));
        }
        else {
            url = QUrl::fromLocalFile(fullFilePath);
        }
#endif
    }

    return url;
}

QString SynthesisPopover::folderPath(const QString &folderId, const QString &filePath)
{
    QString fullFilePath = QString();

    const auto accountInfoIt = _accountInfoMap.find(_currentAccountId);
    if (accountInfoIt != _accountInfoMap.end()) {
        return accountInfoIt->second.folderPath(folderId, filePath);
    }

    return fullFilePath;
}

void SynthesisPopover::openUrl(const QString &folderId, const QString &filePath)
{
    if (!folderId.isEmpty()) {
        QUrl url = folderUrl(folderId, filePath);
        if (!url.isEmpty()) {
            if (!QDesktopServices::openUrl(url)) {
                qCWarning(lcSynthesisPopover) << "QDesktopServices::openUrl failed for " << url.toString();
                QMessageBox msgBox;
                msgBox.setText(tr("Unable to open folder path %1.").arg(url.toString()));
                msgBox.exec();
            }
        }
    }
}

const FolderInfo *SynthesisPopover::getFirstFolderWithStatus(const std::map<QString, FolderInfo *> &folderMap,
                                                             OCC::SyncResult::Status status)
{
    const FolderInfo *folderInfo = nullptr;
    for (auto folderInfoIt : folderMap) {
        if (folderInfoIt.second) {
            if (folderInfoIt.second->_status == status) {
                folderInfo = folderInfoIt.second;
                break;
            }
        }
        else {
            qCDebug(lcSynthesisPopover) << "Null pointer!";
            Q_ASSERT(false);
        }
    }
    return folderInfo;
}

const FolderInfo *SynthesisPopover::getFirstFolderByPriority(const std::map<QString, FolderInfo *> &folderMap)
{
    static QVector<OCC::SyncResult::Status> statusPriority = QVector<OCC::SyncResult::Status>()
            << OCC::SyncResult::Status::NotYetStarted
            << OCC::SyncResult::Status::SyncRunning
            << OCC::SyncResult::Status::Paused
            << OCC::SyncResult::Status::Error
            << OCC::SyncResult::Status::SetupError
            << OCC::SyncResult::Status::Problem
            << OCC::SyncResult::Status::SyncPrepare
            << OCC::SyncResult::Status::Success;

    const FolderInfo *folderInfo = nullptr;
    for (OCC::SyncResult::Status status : statusPriority) {
        folderInfo = getFirstFolderWithStatus(folderMap, status);
        if (folderInfo) {
            break;
        }
    }
    if (!folderInfo) {
        folderInfo = (folderMap.empty() ? nullptr : folderMap.begin()->second);
    }
    return folderInfo;
}

void SynthesisPopover::refreshStatusBar(const FolderInfo *folderInfo)
{
    if (folderInfo) {
        _statusBarWidget->setStatus(folderInfo->_paused, folderInfo->_unresolvedConflicts,
                                    folderInfo->_status, folderInfo->_currentFile,
                                    folderInfo->_totalFiles, folderInfo->_estimatedRemainingTime);
    }
    else {
        _statusBarWidget->reset();
    }
}

void SynthesisPopover::refreshStatusBar(std::map<QString, AccountInfoSynthesis>::iterator accountInfoIt)
{
    if (accountInfoIt != _accountInfoMap.end()) {
        const FolderInfo *folderInfo = getFirstFolderByPriority(accountInfoIt->second._folderMap);
        refreshStatusBar(folderInfo);
    }
}

void SynthesisPopover::refreshStatusBar(QString accountId)
{
    auto accountInfoIt = _accountInfoMap.find(accountId);
    if (accountInfoIt != _accountInfoMap.end()) {
        refreshStatusBar(accountInfoIt);
    }
}

void SynthesisPopover::setSynchronizedDefaultPage(QWidget **widget, QWidget *parent)
{
    static QLabel *defaultTextLabel = nullptr;

    if (!*widget) {
        *widget = new QWidget(parent);

        QVBoxLayout *vboxLayout = new QVBoxLayout();
        vboxLayout->setSpacing(20);
        vboxLayout->addStretch();

        QLabel *iconLabel = new QLabel(parent);
        iconLabel->setAlignment(Qt::AlignHCenter);
        iconLabel->setPixmap(QIcon(":/client/resources/icons/document types/file-default.svg")
                             .pixmap(QSize(defaultLogoIconSize, defaultLogoIconSize)));
        vboxLayout->addWidget(iconLabel);

        QLabel *defaultTitleLabel = new QLabel(tr("No recently synchronized files"), parent);
        defaultTitleLabel->setObjectName("defaultTitleLabel");
        defaultTitleLabel->setAlignment(Qt::AlignHCenter);
        vboxLayout->addWidget(defaultTitleLabel);

        defaultTextLabel = new QLabel(parent);
        defaultTextLabel->setObjectName("defaultTextLabel");
        defaultTextLabel->setAlignment(Qt::AlignHCenter);
        defaultTextLabel->setWordWrap(true);

        vboxLayout->addWidget(defaultTextLabel);

        vboxLayout->addStretch();
        (*widget)->setLayout(vboxLayout);

        connect(defaultTextLabel, &QLabel::linkActivated, this, &SynthesisPopover::onLinkActivated);
    }

    // Set text
    if (defaultTextLabel) {
        OCC::AccountPtr accountPtr = OCC::AccountManager::instance()->getAccountFromId(_currentAccountId);
        if (accountPtr) {
            QUrl accountUrl = accountPtr->url();

            // Get 1st folder...
            OCC::Folder::Map folderMap = OCC::FolderMan::instance()->map();
            QString folderId = QString();
            for (auto folderIt = folderMap.begin(); folderIt != folderMap.end(); folderIt++) {
                if (folderIt.value() && folderIt.value()->accountState()) {
                    OCC::AccountPtr folderAccountPtr = folderIt.value()->accountState()->account();
                    if (!folderAccountPtr.isNull()) {
                        if (folderAccountPtr->id() == accountPtr->id()) {
                            folderId = folderIt.key();
                            break;
                        }
                    }
                    else {
                        qCDebug(lcSynthesisPopover) << "Null pointer!";
                        Q_ASSERT(false);
                    }
                }
                else {
                    qCDebug(lcSynthesisPopover) << "Null pointer!";
                    Q_ASSERT(false);
                }
            }
            if (folderId.isEmpty()) {
                // No folder
                defaultTextLabel->setText(tr("No synchronized folder for this Drive!"));
            }
            else {
                QUrl defaultFolderUrl = folderUrl(folderId, QString());
                defaultTextLabel->setText(tr("You can synchronize files <a style=\"%1\" href=\"%2\">from your computer</a>"
                                             " or on <a style=\"%1\" href=\"%3\">drive.infomaniak.com</a>.")
                                          .arg(OCC::Utility::linkStyle)
                                          .arg(defaultFolderUrl.toString())
                                          .arg(accountUrl.toString()));
            }
        }
        else {
            // No account
            defaultTextLabel->setText(tr("No kDrive configured!"));
        }
    }
    else {
        qCDebug(lcSynthesisPopover) << "Null pointer!";
        Q_ASSERT(false);
    }
}

void SynthesisPopover::onRefreshAccountList()
{
#ifdef CONSOLE_DEBUG
    std::cout << QTime::currentTime().toString("hh:mm:ss").toStdString()
              << " - SynthesisPopover::onRefreshAccountList" << std::endl;
#endif

    if (OCC::AccountManager::instance()->accounts().isEmpty()) {
        _currentAccountId.clear();
        _accountInfoMap.clear();
        _folderButton->setVisible(false);
        _folderButton->setWithMenu(false);
        _webviewButton->setVisible(false);
        _driveSelectionWidget->clear();
        _progressBarWidget->reset();
        _statusBarWidget->reset();
    }
    else {
        bool currentAccountStillExists = !_currentAccountId.isEmpty()
                && OCC::AccountManager::instance()->getAccountFromId(_currentAccountId);
        std::size_t currentFolderMapSize = 0;

        for (OCC::AccountStatePtr accountStatePtr : OCC::AccountManager::instance()->accounts()) {
            if (accountStatePtr && accountStatePtr->account()) {
                QString accountId = accountStatePtr->account()->id();
                auto accountInfoIt = _accountInfoMap.find(accountId);
                if (accountInfoIt == _accountInfoMap.end()) {
                    // New account
                    AccountInfoSynthesis accountInfo(accountStatePtr.data());
                    connect(accountInfo._quotaInfoPtr.get(), &OCC::QuotaInfo::quotaUpdated,
                            this, &SynthesisPopover::onUpdateQuota);

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
                                    accountInfoIt->second._folderMap[folderIt.key()] =
                                            new FolderInfo(folderIt.value()->shortGuiLocalPath(), folderIt.value()->path());
                                    folderInfoIt = accountInfoIt->second._folderMap.find(folderIt.key());
                                }

                                folderInfoIt->second->_paused = folderIt.value()->syncPaused();
                                folderInfoIt->second->_unresolvedConflicts = folderIt.value()->syncResult().hasUnresolvedConflicts();
                                folderInfoIt->second->_status = folderIt.value()->syncResult().status();
                            }
                        }
                        else {
                            qCDebug(lcSynthesisPopover) << "Null pointer!";
                            Q_ASSERT(false);
                        }
                    }
                    else {
                        qCDebug(lcSynthesisPopover) << "Null pointer!";
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

                _driveSelectionWidget->addOrUpdateDrive(accountId, accountInfoIt->second);

                if (_currentAccountId.isEmpty() || !currentAccountStillExists) {
                    _currentAccountId = accountId;
                }

                if (_currentAccountId == accountId) {
                    currentFolderMapSize = accountInfoIt->second._folderMap.size();
                }
            }
            else {
                qCDebug(lcSynthesisPopover) << "Null pointer!";
                Q_ASSERT(false);
            }
        }

        // Manage removed accounts
        auto accountInfoIt = _accountInfoMap.begin();
        while (accountInfoIt != _accountInfoMap.end()) {
            if (!OCC::AccountManager::instance()->getAccountFromId(accountInfoIt->first)) {
                _driveSelectionWidget->removeDrive(accountInfoIt->first);
                accountInfoIt = _accountInfoMap.erase(accountInfoIt);
            }
            else {
                accountInfoIt++;
            }
        }

        _folderButton->setVisible(currentFolderMapSize > 0);
        _folderButton->setWithMenu(currentFolderMapSize > 1);
        _webviewButton->setVisible(currentFolderMapSize > 0);
        _statusBarWidget->setSeveralDrives(_accountInfoMap.size() > 1);
        _driveSelectionWidget->selectDrive(_currentAccountId);
        refreshStatusBar(_currentAccountId);
    }

    setSynchronizedDefaultPage(&_defaultSynchronizedPageWidget, this);
}

void SynthesisPopover::onUpdateProgress(const QString &folderId, const OCC::ProgressInfo &progress)
{
    OCC::Folder *folder = OCC::FolderMan::instance()->folder(folderId);
    if (folder) {
#ifdef CONSOLE_DEBUG
        std::cout << QTime::currentTime().toString("hh:mm:ss").toStdString()
                  << " - SynthesisPopover::onUpdateProgress folder: " << folder->path().toStdString() << std::endl;
#endif

        if (folder->accountState()) {
            OCC::AccountPtr account = folder->accountState()->account();
            if (!account.isNull()) {
                const auto accountInfoIt = _accountInfoMap.find(account->id());
                if (accountInfoIt != _accountInfoMap.end()) {
                    const auto folderInfoIt = accountInfoIt->second._folderMap.find(folderId);
                    if (folderInfoIt != accountInfoIt->second._folderMap.end()) {
                        FolderInfo *folderInfo = folderInfoIt->second;
                        if (folderInfo) {
                            folderInfo->_currentFile = progress.currentFile();
                            folderInfo->_totalFiles = qMax(progress.currentFile(), progress.totalFiles());
                            folderInfo->_completedSize = progress.completedSize();
                            folderInfo->_totalSize = qMax(progress.completedSize(), progress.totalSize());
                            folderInfo->_estimatedRemainingTime = progress.totalProgress().estimatedEta;
                            folderInfo->_paused = folder->syncPaused();
                            folderInfo->_unresolvedConflicts = folder->syncResult().hasUnresolvedConflicts();
                            folderInfo->_status = folder->syncResult().status();
                        }
                        else {
                            qCDebug(lcSynthesisPopover) << "Null pointer!";
                            Q_ASSERT(false);
                        }

                        if (account->id() == _currentAccountId) {
                            refreshStatusBar(folderInfo);
                        }
                    }

                    // Compute account status
                    accountInfoIt->second.updateStatus();

                    _driveSelectionWidget->addOrUpdateDrive(account->id(), accountInfoIt->second);
                }
            }
            else {
                qCDebug(lcSynthesisPopover) << "Null pointer!";
                Q_ASSERT(false);
            }
        }
        else {
            qCDebug(lcSynthesisPopover) << "Null pointer!";
            Q_ASSERT(false);
        }
    }
}

void SynthesisPopover::onUpdateQuota(qint64 total, qint64 used)
{
    QString accountId = qvariant_cast<QString>(sender()->property(accountIdProperty));

#ifdef CONSOLE_DEBUG
    std::cout << QTime::currentTime().toString("hh:mm:ss").toStdString()
              << " - SynthesisPopover::onUpdateQuota account: " << accountId.toStdString() << std::endl;
#endif

    const auto accountInfoIt = _accountInfoMap.find(accountId);
    if (accountInfoIt != _accountInfoMap.end()) {
        accountInfoIt->second._totalSize = total;
        accountInfoIt->second._used = used;

        if (accountId == _currentAccountId) {
            _progressBarWidget->setUsedSize(total, used);
        }
    }
}

void SynthesisPopover::onItemCompleted(const QString &folderId, const OCC::SyncFileItemPtr &item)
{
#ifdef CONSOLE_DEBUG
    std::cout << QTime::currentTime().toString("hh:mm:ss").toStdString()
              << " - SynthesisPopover::onItemCompleted" << std::endl;
#endif

    if (!item.isNull()) {
        if (item.data()->_status == OCC::SyncFileItem::NoStatus
                || item.data()->_status == OCC::SyncFileItem::FatalError
                || item.data()->_status == OCC::SyncFileItem::NormalError
                || item.data()->_status == OCC::SyncFileItem::SoftError
                || item.data()->_status == OCC::SyncFileItem::DetailError
                || item.data()->_status == OCC::SyncFileItem::BlacklistedError
                || item.data()->_status == OCC::SyncFileItem::FileIgnored) {
            return;
        }

        OCC::Folder *folder = OCC::FolderMan::instance()->folder(folderId);
        if (folder) {
            if (folder->accountState()) {
                OCC::AccountPtr account = folder->accountState()->account();
                if (!account.isNull()) {
                    const auto accountInfoIt = _accountInfoMap.find(account->id());
                    if (accountInfoIt != _accountInfoMap.end()) {
                        if (!accountInfoIt->second._synchronizedListWidget) {
                            accountInfoIt->second._synchronizedListWidget = new QListWidget(this);
                            accountInfoIt->second._synchronizedListWidget->setSpacing(0);
                            accountInfoIt->second._synchronizedListWidget->setSelectionMode(QAbstractItemView::NoSelection);
                            accountInfoIt->second._synchronizedListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
                            accountInfoIt->second._synchronizedListStackPosition =
                                    _stackedWidget->addWidget(accountInfoIt->second._synchronizedListWidget);
                            if (_currentAccountId == accountInfoIt->first
                                    && _buttonsBarWidget->position() == StackedWidget::Synchronized) {
                                _stackedWidget->setCurrentIndex(accountInfoIt->second._synchronizedListStackPosition);
                            }
                        }

                        // Add item to synchronized list
                        QListWidgetItem *widgetItem = new QListWidgetItem();
                        SynchronizedItem synchronizedItem(folderId,
                                                          item.data()->_file,
                                                          item.data()->_fileId,
                                                          item.data()->_status,
                                                          item.data()->_direction,
                                                          folderPath(folderId, item.data()->_file),
                                                          QDateTime::currentDateTime());
                        accountInfoIt->second._synchronizedListWidget->insertItem(0, widgetItem);
                        SynchronizedItemWidget *widget = new SynchronizedItemWidget(synchronizedItem,
                                                                                    accountInfoIt->second._synchronizedListWidget);
                        accountInfoIt->second._synchronizedListWidget->setItemWidget(widgetItem, widget);
                        connect(widget, &SynchronizedItemWidget::openFolder, this, &SynthesisPopover::onOpenFolderItem);
                        connect(widget, &SynchronizedItemWidget::open, this, &SynthesisPopover::onOpenItem);
                        connect(widget, &SynchronizedItemWidget::addToFavourites, this, &SynthesisPopover::onAddToFavouriteItem);
                        connect(widget, &SynchronizedItemWidget::manageRightAndSharing, this, &SynthesisPopover::onManageRightAndSharingItem);
                        connect(widget, &SynchronizedItemWidget::copyLink, this, &SynthesisPopover::onCopyLinkItem);
                        connect(widget, &SynchronizedItemWidget::displayOnWebview, this, &SynthesisPopover::onOpenWebviewItem);

                        if (accountInfoIt->second._synchronizedListWidget->count() > maxSynchronizedItems) {
                            // Remove last row
                            QListWidgetItem *lastWidgetItem = accountInfoIt->second._synchronizedListWidget->takeItem(
                                        accountInfoIt->second._synchronizedListWidget->count() - 1);
                            delete lastWidgetItem;
                        }
                    }
                }
                else {
                    qCDebug(lcSynthesisPopover) << "Null pointer!";
                    Q_ASSERT(false);
                }
            }
            else {
                qCDebug(lcSynthesisPopover) << "Null pointer!";
                Q_ASSERT(false);
            }
        }
    }
    else {
        qCDebug(lcSynthesisPopover) << "Null pointer!";
        Q_ASSERT(false);
    }
}

void SynthesisPopover::onOpenFolderMenu(bool checked)
{
    Q_UNUSED(checked)

    auto accountInfoIt = _accountInfoMap.find(_currentAccountId);
    if (accountInfoIt != _accountInfoMap.end()) {
        if (accountInfoIt->second._folderMap.size() == 1) {
            // Open folder
            auto folderInfoIt = accountInfoIt->second._folderMap.begin();
            openUrl(folderInfoIt->first);
        }
        else if (accountInfoIt->second._folderMap.size() > 1) {
            // Open menu
            MenuWidget *menu = new MenuWidget(MenuWidget::Menu, this);
            for (auto folderInfoIt : accountInfoIt->second._folderMap) {
                QWidgetAction *openFolderAction = new QWidgetAction(this);
                openFolderAction->setProperty(MenuWidget::actionTypeProperty.c_str(), folderInfoIt.first);
                MenuItemWidget *openFolderMenuItemWidget = new MenuItemWidget(folderInfoIt.second->_name);
                openFolderMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/folder.svg");
                openFolderAction->setDefaultWidget(openFolderMenuItemWidget);
                connect(openFolderAction, &QWidgetAction::triggered, this, &SynthesisPopover::onOpenFolder);
                menu->addAction(openFolderAction);
            }
            menu->exec(QWidget::mapToGlobal(_folderButton->geometry().center()));
        }
    }
}

void SynthesisPopover::onOpenFolder(bool checked)
{
    Q_UNUSED(checked)

    QString folderId = qvariant_cast<QString>(sender()->property(MenuWidget::actionTypeProperty.c_str()));
    openUrl(folderId);
}

void SynthesisPopover::onOpenWebview(bool checked)
{
    Q_UNUSED(checked)

    OCC::AccountPtr accountPtr = OCC::AccountManager::instance()->getAccountFromId(_currentAccountId);
    if (accountPtr) {
        if (!QDesktopServices::openUrl(accountPtr->url())) {
            qCWarning(lcSynthesisPopover) << "QDesktopServices::openUrl failed for " << accountPtr->url().toString();
            QMessageBox msgBox;
            msgBox.setText(tr("Unable to access web site %1.").arg(accountPtr->url().toString()));
            msgBox.exec();
        }
    }
}

void SynthesisPopover::onOpenMiscellaneousMenu(bool checked)
{
    Q_UNUSED(checked)

    MenuWidget *menu = new MenuWidget(MenuWidget::Menu, this);

    // Parameters
    QWidgetAction *parametersAction = new QWidgetAction(this);
    MenuItemWidget *parametersMenuItemWidget = new MenuItemWidget(tr("Parameters"));
    parametersMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/parameters.svg");
    parametersAction->setDefaultWidget(parametersMenuItemWidget);
    connect(parametersAction, &QWidgetAction::triggered, this, &SynthesisPopover::onOpenParameters);
    menu->addAction(parametersAction);

    // Disable Notifications
    QWidgetAction *notificationsAction = new QWidgetAction(this);
    bool notificationAlreadyDisabledForPeriod = _notificationsDisabled != NotificationsDisabled::Never
            && _notificationsDisabled != NotificationsDisabled::Always;
    MenuItemWidget *notificationsMenuItemWidget = new MenuItemWidget(
                notificationAlreadyDisabledForPeriod
                ? tr("Notifications disabled until %1").arg(_notificationsDisabledUntilDateTime.toString())
                : tr("Disable Notifications"));
    notificationsMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/notification-off.svg");
    notificationsMenuItemWidget->setHasSubmenu(true);
    notificationsAction->setDefaultWidget(notificationsMenuItemWidget);
    menu->addAction(notificationsAction);

    // Disable Notifications submenu
    MenuWidget *submenu = new MenuWidget(MenuWidget::Submenu, menu);

    QActionGroup *notificationActionGroup = new QActionGroup(this);
    notificationActionGroup->setExclusive(true);

    const std::map<NotificationsDisabled, QString> &notificationMap =
            _notificationsDisabled == NotificationsDisabled::Never
            || _notificationsDisabled == NotificationsDisabled::Always
            ? _notificationsDisabledMap
            : _notificationsDisabledForPeriodMap;

    QWidgetAction *notificationAction;
    for (auto notificationActionsItem : notificationMap) {
        notificationAction = new QWidgetAction(this);
        notificationAction->setProperty(MenuWidget::actionTypeProperty.c_str(), notificationActionsItem.first);
        MenuItemWidget *notificationMenuItemWidget = new MenuItemWidget(notificationActionsItem.second);
        notificationMenuItemWidget->setChecked(notificationActionsItem.first == _notificationsDisabled);
        notificationAction->setDefaultWidget(notificationMenuItemWidget);
        connect(notificationAction, &QWidgetAction::triggered, this, &SynthesisPopover::onNotificationActionTriggered);
        notificationActionGroup->addAction(notificationAction);
    }

    submenu->addActions(notificationActionGroup->actions());
    notificationsAction->setMenu(submenu);

    // Help
    QWidgetAction *helpAction = new QWidgetAction(this);
    MenuItemWidget *helpMenuItemWidget = new MenuItemWidget(tr("Need help"));
    helpMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/help.svg");
    helpAction->setDefaultWidget(helpMenuItemWidget);
    connect(helpAction, &QWidgetAction::triggered, this, &SynthesisPopover::onDisplayHelp);
    menu->addAction(helpAction);

    // Quit
    QWidgetAction *exitAction = new QWidgetAction(this);
    MenuItemWidget *exitMenuItemWidget = new MenuItemWidget(tr("Quit application"));
    exitMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/error-sync.svg");
    exitAction->setDefaultWidget(exitMenuItemWidget);
    connect(exitAction, &QWidgetAction::triggered, this, &SynthesisPopover::onExit);
    menu->addAction(exitAction);

    if (_debugMode) {
        // Test crash
        QWidgetAction *crashAction = new QWidgetAction(this);
        MenuItemWidget *crashMenuItemWidget = new MenuItemWidget("Test crash");
        crashAction->setDefaultWidget(crashMenuItemWidget);
        connect(crashAction, &QWidgetAction::triggered, this, &SynthesisPopover::onCrash);
        menu->addAction(crashAction);

        // Test crash enforce
        QWidgetAction *crashEnforceAction = new QWidgetAction(this);
        MenuItemWidget *crashEnforceMenuItemWidget = new MenuItemWidget("Test crash enforce");
        crashEnforceAction->setDefaultWidget(crashEnforceMenuItemWidget);
        connect(crashEnforceAction, &QWidgetAction::triggered, this, &SynthesisPopover::onCrashEnforce);
        menu->addAction(crashEnforceAction);

        // Test crash fatal
        QWidgetAction *crashFatalAction = new QWidgetAction(this);
        MenuItemWidget *crashFatalMenuItemWidget = new MenuItemWidget("Test crash fatal");
        crashFatalAction->setDefaultWidget(crashFatalMenuItemWidget);
        connect(crashFatalAction, &QWidgetAction::triggered, this, &SynthesisPopover::onCrashFatal);
        menu->addAction(crashFatalAction);
    }

    menu->exec(QWidget::mapToGlobal(_menuButton->geometry().center()));
}

void SynthesisPopover::onOpenParameters(bool checked)
{
    Q_UNUSED(checked)

    emit openParametersDialog();
}

void SynthesisPopover::onDisplayHelp(bool checked)
{
    Q_UNUSED(checked)

    QDesktopServices::openUrl(QUrl(OCC::Theme::instance()->helpUrl()));
}

void SynthesisPopover::onExit(bool checked)
{
    Q_UNUSED(checked)

    emit exit();
}

void SynthesisPopover::onCrash(bool checked)
{
    Q_UNUSED(checked)

    emit crash();
}

void SynthesisPopover::onCrashEnforce(bool checked)
{
    Q_UNUSED(checked)

    emit crashEnforce();
}

void SynthesisPopover::onCrashFatal(bool checked)
{
    Q_UNUSED(checked)

    emit crashFatal();
}

void SynthesisPopover::onNotificationActionTriggered(bool checked)
{
    Q_UNUSED(checked)

    QString message;
    bool notificationAlreadyDisabledForPeriod = _notificationsDisabled != NotificationsDisabled::Never
            && _notificationsDisabled != NotificationsDisabled::Always;

    _notificationsDisabled = qvariant_cast<NotificationsDisabled>(sender()->property(MenuWidget::actionTypeProperty.c_str()));
    switch (_notificationsDisabled) {
    case NotificationsDisabled::Never:
        _notificationsDisabledUntilDateTime = QDateTime();
        message = QString(tr("Notifications enabled!"));
        break;
    case NotificationsDisabled::OneHour:
        _notificationsDisabledUntilDateTime = notificationAlreadyDisabledForPeriod
                ? _notificationsDisabledUntilDateTime.addSecs(60 * 60)
                : QDateTime::currentDateTime().addSecs(60 * 60);
        message = QString(tr("Notifications disabled until %1").arg(_notificationsDisabledUntilDateTime.toString()));
        break;
    case NotificationsDisabled::UntilTomorrow:
        _notificationsDisabledUntilDateTime = QDateTime(QDateTime::currentDateTime().addDays(1).date(), QTime(8, 0));
        message = QString(tr("Notifications disabled until %1").arg(_notificationsDisabledUntilDateTime.toString()));
        break;
    case NotificationsDisabled::TreeDays:
        _notificationsDisabledUntilDateTime = notificationAlreadyDisabledForPeriod
                ? _notificationsDisabledUntilDateTime.addDays(3)
                : QDateTime::currentDateTime().addDays(3);
        message = QString(tr("Notifications disabled until %1").arg(_notificationsDisabledUntilDateTime.toString()));
        break;
    case NotificationsDisabled::OneWeek:
        _notificationsDisabledUntilDateTime = notificationAlreadyDisabledForPeriod
                ? _notificationsDisabledUntilDateTime.addDays(7)
                : QDateTime::currentDateTime().addDays(7);
        message = QString(tr("Notifications disabled until %1").arg(_notificationsDisabledUntilDateTime.toString()));
        break;
    case NotificationsDisabled::Always:
        _notificationsDisabledUntilDateTime = QDateTime();
        message = QString(tr("Notifications disabled!"));
        break;
    }

    emit disableNotifications(_notificationsDisabled, _notificationsDisabledUntilDateTime);

    QMessageBox msgBox;
    msgBox.setText(message);
    msgBox.exec();
}

void SynthesisPopover::onAccountSelected(QString id)
{
    const auto accountInfoIt = _accountInfoMap.find(id);
    if (accountInfoIt != _accountInfoMap.end()) {
        _currentAccountId = id;

        std::size_t currentFolderMapSize = accountInfoIt->second._folderMap.size();
        _folderButton->setVisible(currentFolderMapSize > 0);
        _folderButton->setWithMenu(currentFolderMapSize > 1);
        _progressBarWidget->setUsedSize(accountInfoIt->second._totalSize, accountInfoIt->second._used);
        refreshStatusBar(accountInfoIt);
        setSynchronizedDefaultPage(&_defaultSynchronizedPageWidget, this);
        _buttonsBarWidget->selectButton(int(accountInfoIt->second._stackedWidgetPosition));
    }
}

void SynthesisPopover::onAddDrive()
{
    emit addDrive();
}

void SynthesisPopover::onPauseSync(bool all)
{
    OCC::Utility::pauseSync(all ? QString() : _currentAccountId, true);
}

void SynthesisPopover::onResumeSync(bool all)
{
    OCC::Utility::pauseSync(all ? QString() : _currentAccountId, false);
}

void SynthesisPopover::onRunSync(bool all)
{
    OCC::Utility::runSync(all ? QString() : _currentAccountId);
}

void SynthesisPopover::onButtonBarToggled(int position)
{
    const auto accountInfoIt = _accountInfoMap.find(_currentAccountId);
    if (accountInfoIt != _accountInfoMap.end()) {
        accountInfoIt->second._stackedWidgetPosition = StackedWidget(position);
    }

    switch (position) {
    case StackedWidget::Synchronized:
        if (accountInfoIt != _accountInfoMap.end()
                && accountInfoIt->second._synchronizedListStackPosition) {
            _stackedWidget->setCurrentIndex(accountInfoIt->second._synchronizedListStackPosition);
        }
        else {
            _stackedWidget->setCurrentIndex(StackedWidget::Synchronized);
        }
        break;
    case StackedWidget::Favorites:
        if (accountInfoIt != _accountInfoMap.end()
                && accountInfoIt->second._favoritesListStackPosition) {
            _stackedWidget->setCurrentIndex(accountInfoIt->second._favoritesListStackPosition);
        }
        else {
            _stackedWidget->setCurrentIndex(StackedWidget::Favorites);
        }
        break;
    case StackedWidget::Activity:
        if (accountInfoIt != _accountInfoMap.end() &&
                accountInfoIt->second._activityListStackPosition) {
            _stackedWidget->setCurrentIndex(accountInfoIt->second._activityListStackPosition);
        }
        else {
            _stackedWidget->setCurrentIndex(StackedWidget::Activity);
        }
        break;
    }
}

void SynthesisPopover::onOpenFolderItem(const SynchronizedItem &item)
{
    QString fullFilePath = folderPath(item.folderId(), item.filePath());
    if (!fullFilePath.isEmpty()) {
        OCC::showInFileManager(fullFilePath);
    }
}

void SynthesisPopover::onOpenItem(const SynchronizedItem &item)
{
    openUrl(item.folderId(), item.filePath());
}

void SynthesisPopover::onAddToFavouriteItem(const SynchronizedItem &item)
{
    Q_UNUSED(item)

    QMessageBox msgBox;
    msgBox.setText(tr("Not implemented!"));
    msgBox.exec();
}

void SynthesisPopover::onManageRightAndSharingItem(const SynchronizedItem &item)
{
    QString folderRelativePath;
    QString fullFilePath = folderPath(item.folderId(), item.filePath());
    OCC::FolderMan::instance()->folderForPath(fullFilePath, &folderRelativePath);
    if (folderRelativePath == "/") {
        qCDebug(lcSynthesisPopover) << "Cannot share root directory!";
        QMessageBox msgBox;
        msgBox.setText(tr("You cannot share the root directory of your Drive!"));
        msgBox.exec();
    }
    else {
        emit openShareDialogPublicLinks(folderRelativePath, fullFilePath);
    }
}

void SynthesisPopover::onCopyLinkItem(const SynchronizedItem &item)
{
    QString folderRelativePath;
    QString fullFilePath = folderPath(item.folderId(), item.filePath());
    OCC::FolderMan::instance()->folderForPath(fullFilePath, &folderRelativePath);

    OCC::Folder *folder = OCC::FolderMan::instance()->folder(item.folderId());
    if (folder) {
        QString serverRelativePath = QDir(folder->remotePath()).filePath(folderRelativePath);

        OCC::AccountPtr accountPtr = OCC::AccountManager::instance()->getAccountFromId(_currentAccountId);
        if (!accountPtr.isNull()) {
            auto job = new OCC::GetOrCreatePublicLinkShare(accountPtr, serverRelativePath, this);
            connect(job, &OCC::GetOrCreatePublicLinkShare::done, this, &SynthesisPopover::onCopyUrlToClipboard);
            connect(job, &OCC::GetOrCreatePublicLinkShare::error, this,
                    [=]() { emit openShareDialogPublicLinks(folderRelativePath, fullFilePath); });

            job->run();
        }
        else {
            qCDebug(lcSynthesisPopover) << "Null pointer!";
            Q_ASSERT(false);
        }
    }
}

void SynthesisPopover::onOpenWebviewItem(const SynchronizedItem &item)
{
    OCC::AccountPtr accountPtr = OCC::AccountManager::instance()->getAccountFromId(_currentAccountId);
    OCC::fetchPrivateLinkUrl(accountPtr, item.filePath(), item.fileId(), this,
                             [this](const QString &url) { OCC::Utility::openBrowser(url, this); });
}

void SynthesisPopover::onCopyUrlToClipboard(const QString &url)
{
    QApplication::clipboard()->setText(url);
    QMessageBox msgBox;
    msgBox.setText(tr("The shared link has been copied to the clipboard."));
    msgBox.exec();
}

void SynthesisPopover::onLinkActivated(const QString &link)
{
    if (link == OCC::Utility::learnMoreLink) {
        // TODO: add parameters
        onOpenParameters();
    }
    else {
        // URL link
        QUrl url = QUrl(link);
        if (url.isValid()) {
            if (!QDesktopServices::openUrl(QUrl(link))) {
                qCWarning(lcSynthesisPopover) << "QDesktopServices::openUrl failed for " << link;
                QMessageBox msgBox;
                msgBox.setText(tr("Unable to open link %1.").arg(link));
                msgBox.exec();
            }
        }
        else {
            qCWarning(lcSynthesisPopover) << "Invalid link " << link;
            QMessageBox msgBox;
            msgBox.setText(tr("Invalid link %1.").arg(link));
            msgBox.exec();
        }
    }
}

SynthesisPopover::AccountInfoSynthesis::AccountInfoSynthesis()
    : AccountInfo()
    , _stackedWidgetPosition(StackedWidget::Synchronized)
    , _synchronizedListWidget(nullptr)
    , _synchronizedListStackPosition(StackedWidget::Synchronized)
    , _favoritesListStackPosition(StackedWidget::Favorites)
    , _activityListStackPosition(StackedWidget::Activity)
{
}

SynthesisPopover::AccountInfoSynthesis::AccountInfoSynthesis(OCC::AccountState *accountState)
    : AccountInfo(accountState)
    , _stackedWidgetPosition(StackedWidget::Synchronized)
    , _synchronizedListWidget(nullptr)
    , _synchronizedListStackPosition(StackedWidget::Synchronized)
    , _favoritesListStackPosition(StackedWidget::Favorites)
    , _activityListStackPosition(StackedWidget::Activity)
{
}

}
