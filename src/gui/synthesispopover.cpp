#include "synthesispopover.h"
#include "menuitemwidget.h"
#include "menuwidget.h"
#include "buttonsbarwidget.h"
#include "bottomwidget.h"
#include "custompushbutton.h"
#include "synchronizeditem.h"
#include "account.h"
#include "common/utility.h"
#include "guiutility.h"
#include "openfilemanager.h"
#include "networkjobs.h"
#include "socketapi.h"
#include "getorcreatepubliclinkshare.h"

#include <iostream>

#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QClipboard>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QGraphicsDropShadowEffect>
#include <QGuiApplication>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QPicture>
#include <QScreen>
#include <QVector>
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
static const int maxSynchronizedItems = 10;
static const char accountIdProperty[] = "accountId";

Q_LOGGING_CATEGORY(lcSynthesisPopover, "synthesispopover", QtInfoMsg)

SynthesisPopover::SynthesisPopover(bool debugMode, QRect sysrayIconRect, QWidget *parent)
    : QDialog(parent)
    , _debugMode(debugMode)
    , _sysTrayIconRect(sysrayIconRect)
    , _currentAccountId(QString())
    , _backgroundMainColor(QColor())
    , _folderButton(nullptr)
    , _webviewButton(nullptr)
    , _menuButton(nullptr)
    , _driveSelectionWidget(nullptr)
    , _progressBarWidget(nullptr)
    , _statusBarWidget(nullptr)
    , _stackedWidget(nullptr)
    , _notificationAction(notificationActions::Never)
{
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::X11BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground);

    initUI();

    connect(this, &SynthesisPopover::refreshAccountList, this, &SynthesisPopover::onRefreshAccountList);
    connect(this, &SynthesisPopover::updateProgress, this, &SynthesisPopover::onUpdateProgress);
    connect(this, &SynthesisPopover::itemCompleted, this, &SynthesisPopover::onItemCompleted);
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
    if (event->type() == QEvent::WindowDeactivate) {
        done(QDialog::Accepted);
        event->ignore();
        return true;
    }
    return QWidget::event(event);
}

void SynthesisPopover::initUI()
{
    QVBoxLayout *mainVBox = new QVBoxLayout();
    mainVBox->setContentsMargins(triangleHeight, triangleHeight, triangleHeight, triangleHeight);
    mainVBox->setSpacing(0);

    // Tool bar
    QHBoxLayout *hboxToolBar = new QHBoxLayout();
    hboxToolBar->setContentsMargins(toolBarHMargin, toolBarVMargin, toolBarHMargin, toolBarVMargin);
    hboxToolBar->setSpacing(toolBarSpacing);
    mainVBox->addLayout(hboxToolBar);

    QLabel *iconLabel = new QLabel(this);
    iconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/logos/kdrive-without-text.svg")
                         .pixmap(logoIconSize, logoIconSize));
    hboxToolBar->addWidget(iconLabel);

    QWidget *spacerWidget = new QWidget(this);
    spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    hboxToolBar->addWidget(spacerWidget);

    _folderButton = new CustomToolButton(this);
    _folderButton->setIconPath(":/client/resources/icons/actions/folder.svg");
    _folderButton->setToolTip(tr("Open local folder"));
    hboxToolBar->addWidget(_folderButton);

    _webviewButton = new CustomToolButton(this);
    _webviewButton->setIconPath(":/client/resources/icons/actions/webview.svg");
    _webviewButton->setToolTip(tr("Open in browser"));
    hboxToolBar->addWidget(_webviewButton);

    _menuButton = new CustomToolButton(this);
    _menuButton->setIconPath(":/client/resources/icons/actions/menu.svg");
    hboxToolBar->addWidget(_menuButton);

    // Drive selection
    QHBoxLayout *hboxDriveBar = new QHBoxLayout();
    hboxDriveBar->setContentsMargins(driveBarHMargin, driveBarVMargin, driveBarHMargin, driveBarVMargin);
    hboxDriveBar->setSpacing(driveBarSpacing);

    _driveSelectionWidget = new DriveSelectionWidget(this);
    hboxDriveBar->addWidget(_driveSelectionWidget);

    hboxDriveBar->addStretch();
    mainVBox->addLayout(hboxDriveBar);

    // Progress bar
    _progressBarWidget = new ProgressBarWidget(this);
    mainVBox->addWidget(_progressBarWidget);

    // Status bar
    _statusBarWidget = new StatusBarWidget(this);
    mainVBox->addWidget(_statusBarWidget);

    // Buttons bar
    ButtonsBarWidget *buttonsBarWidget = new ButtonsBarWidget(this);

    CustomPushButton *synchronizedButton = new CustomPushButton(tr("Synchronized"), buttonsBarWidget);
    synchronizedButton->setIconPath(":/client/resources/icons/actions/sync.svg");
    buttonsBarWidget->insertButton(stackedWidget::Synchronized, synchronizedButton);

    CustomPushButton *favoritesButton = new CustomPushButton(tr("Favorites"), buttonsBarWidget);
    favoritesButton->setIconPath(":/client/resources/icons/actions/favorite.svg");
    buttonsBarWidget->insertButton(stackedWidget::Favorites, favoritesButton);

    CustomPushButton *activityButton = new CustomPushButton(tr("Activity"), buttonsBarWidget);
    activityButton->setIconPath(":/client/resources/icons/actions/notifications.svg");
    buttonsBarWidget->insertButton(stackedWidget::Activity, activityButton);

    mainVBox->addWidget(buttonsBarWidget);

    // Stacked widget
    _stackedWidget = new QStackedWidget(this);

    QLabel *noDriveLabel = new QLabel(tr("No drive selected!"), this);
    noDriveLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    _stackedWidget->insertWidget(stackedWidget::Synchronized, noDriveLabel);

    QLabel *notImplementedLabel = new QLabel(tr("Not implemented!"), this);
    notImplementedLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    _stackedWidget->insertWidget(stackedWidget::Favorites, notImplementedLabel);

    QLabel *notImplementedLabel2 = new QLabel(tr("Not implemented!"), this);
    notImplementedLabel2->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    _stackedWidget->insertWidget(stackedWidget::Activity, notImplementedLabel2);

    mainVBox->addWidget(_stackedWidget);
    mainVBox->setStretchFactor(_stackedWidget, 1);

    // Bottom
    BottomWidget *bottomWidget = new BottomWidget(this);
    mainVBox->addWidget(bottomWidget);

    setLayout(mainVBox);

    // Shadow
    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect;
    effect->setBlurRadius(shadowBlurRadius);
    effect->setOffset(0);
    setGraphicsEffect(effect);

    connect(_folderButton, &CustomToolButton::clicked, this, &SynthesisPopover::onOpenFolderMenu);
    connect(_webviewButton, &CustomToolButton::clicked, this, &SynthesisPopover::onOpenWebview);
    connect(_menuButton, &CustomToolButton::clicked, this, &SynthesisPopover::onOpenMenu);
    connect(_driveSelectionWidget, &DriveSelectionWidget::driveSelected, this, &SynthesisPopover::onAccountSelected);
    connect(_statusBarWidget, &StatusBarWidget::pauseSync, this, &SynthesisPopover::onPauseSync);
    connect(_statusBarWidget, &StatusBarWidget::resumeSync, this, &SynthesisPopover::onResumeSync);
    connect(_statusBarWidget, &StatusBarWidget::runSync, this, &SynthesisPopover::onRunSync);
    connect(buttonsBarWidget, &ButtonsBarWidget::buttonToggled, this, &SynthesisPopover::onButtonBarToggled);
}

OCC::SyncResult::Status SynthesisPopover::computeAccountStatus(const std::map<QString, SynthesisPopover::FolderInfo> &folderMap)
{
    OCC::SyncResult::Status status = OCC::SyncResult::Undefined;

    int cnt = folderMap.size();

    if (cnt == 1) {
        FolderInfo folderInfo = folderMap.begin()->second;
        if (folderInfo._syncPaused) {
            status = OCC::SyncResult::Paused;
        } else {
            switch (folderInfo._status) {
            case OCC::SyncResult::Undefined:
                status = OCC::SyncResult::Error;
                break;
            case OCC::SyncResult::Problem:
                status = OCC::SyncResult::Success;
                break;
            default:
                status = folderInfo._status;
                break;
            }
        }
    } else {
        int errorsSeen = 0;
        int goodSeen = 0;
        int abortOrPausedSeen = 0;
        int runSeen = 0;
        int various = 0;

        for (auto it = folderMap.begin(); it != folderMap.end(); it++) {
            FolderInfo folderInfo = it->second;
            if (folderInfo._syncPaused) {
                abortOrPausedSeen++;
            } else {
                switch (folderInfo._status) {
                case OCC::SyncResult::Undefined:
                case OCC::SyncResult::NotYetStarted:
                    various++;
                    break;
                case OCC::SyncResult::SyncPrepare:
                case OCC::SyncResult::SyncRunning:
                    runSeen++;
                    break;
                case OCC::SyncResult::Problem:
                case OCC::SyncResult::Success:
                    goodSeen++;
                    break;
                case OCC::SyncResult::Error:
                case OCC::SyncResult::SetupError:
                    errorsSeen++;
                    break;
                case OCC::SyncResult::SyncAbortRequested:
                case OCC::SyncResult::Paused:
                    abortOrPausedSeen++;
                }
            }
        }
        if (errorsSeen > 0) {
            status = OCC::SyncResult::Error;
        } else if (abortOrPausedSeen > 0 && abortOrPausedSeen == cnt) {
            // only if all folders are paused
            status = OCC::SyncResult::Paused;
        } else if (runSeen > 0) {
            status = OCC::SyncResult::SyncRunning;
        } else if (goodSeen > 0) {
            status = OCC::SyncResult::Success;
        }
    }
    return status;
}

void SynthesisPopover::computeAccountProgress(const std::map<QString, SynthesisPopover::FolderInfo> &folderMap,
                                            qint64 &currentFile, qint64 &totalFiles,
                                            qint64 &completedSize, qint64 &totalSize,
                                            qint64 &estimatedRemainingTime)
{
    currentFile = 0;
    totalFiles = 0;
    completedSize = 0;
    totalSize = 0;
    estimatedRemainingTime = 0;
    for (auto folderIt = folderMap.begin(); folderIt != folderMap.end(); folderIt++) {
        currentFile += folderIt->second._currentFile;
        totalFiles += folderIt->second._totalFiles;
        completedSize += folderIt->second._completedSize;
        totalSize += folderIt->second._totalSize;
        estimatedRemainingTime += folderIt->second._estimatedRemainingTime;
    }
}

void SynthesisPopover::pauseSync(bool pause)
{
    // (Un)pause all the folders of the drive
    OCC::FolderMan *folderMan = OCC::FolderMan::instance();
    for (auto folder : folderMan->map()) {
        OCC::AccountPtr folderAccount = folder->accountState()->account();
        if (folderAccount->id() == _currentAccountId) {
            folder->setSyncPaused(pause);
            if (pause) {
                folder->slotTerminateSync();
            }
        }
    }
}

QUrl SynthesisPopover::folderUrl(const QString &folderId, const QString &filePath)
{
    QUrl url;
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
    QString fullFilePath;

    const auto accountStatusIt = _accountStatusMap.find(_currentAccountId);
    if (accountStatusIt != _accountStatusMap.end()) {
        const auto folderInfoIt = accountStatusIt->second._folderMap.find(folderId);
        if (folderInfoIt != accountStatusIt->second._folderMap.end()) {
            fullFilePath = folderInfoIt->second._path + filePath;
            if (!QFile::exists(fullFilePath)) {
                qCWarning(lcSynthesisPopover) << "Invalid path " << fullFilePath;
                fullFilePath = QString();
            }
        }
    }

    return fullFilePath;
}

void SynthesisPopover::openUrl(const QString &folderId, const QString &filePath)
{
    if (!folderId.isEmpty()) {
        QUrl url = folderUrl(folderId, filePath);
        if (!url.isEmpty()) {
            if (!QDesktopServices::openUrl(url)) {
                qCWarning(lcSynthesisPopover) << "QDesktopServices::openUrl failed for " << url;
            }
        }
    }
}

const SynchronizedItem *SynthesisPopover::currentSynchronizedItem()
{
    const auto accountStatusIt = _accountStatusMap.find(_currentAccountId);
    if (accountStatusIt != _accountStatusMap.end()) {
        if (accountStatusIt->second._synchronizedListWidget && accountStatusIt->second._currentSynchronizedWidgetItem) {
            SynchronizedItemWidget *synchronizedItemWidget = dynamic_cast<SynchronizedItemWidget *>(
                        accountStatusIt->second._synchronizedListWidget->itemWidget(
                            accountStatusIt->second._currentSynchronizedWidgetItem));
            return synchronizedItemWidget->item();
        }
    }
    return nullptr;
}

void SynthesisPopover::onRefreshAccountList()
{
    std::cout << "onRefreshAccountList" << std::endl;

    if (OCC::AccountManager::instance()->accounts().isEmpty()) {
        _currentAccountId.clear();
        _accountStatusMap.clear();
        _driveSelectionWidget->clear();
        _progressBarWidget->reset();
        _statusBarWidget->reset();
    }
    else {
        bool currentAccountStillExists = !_currentAccountId.isEmpty()
                && OCC::AccountManager::instance()->getAccountFromId(_currentAccountId);

        for (OCC::AccountStatePtr accountStatePtr : OCC::AccountManager::instance()->accounts()) {
            QString accountId = accountStatePtr->account()->id();
            auto accountStatusIt = _accountStatusMap.find(accountId);
            if (accountStatusIt == _accountStatusMap.end()) {
                // New account
                AccountStatus accountStatus(accountStatePtr.data());

                accountStatus._synchronizedListWidget = new QListWidget(this);
                accountStatus._synchronizedListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
                accountStatus._synchronizedListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
                accountStatus._synchronizedListWidget->setSpacing(0);
                connect(accountStatus._synchronizedListWidget, &QListWidget::currentItemChanged,
                        this, &SynthesisPopover::onCurrentSynchronizedWidgetItemChanged);
                accountStatus._synchronizedListStackPosition = _stackedWidget->addWidget(accountStatus._synchronizedListWidget);

                connect(accountStatus._quotaInfoPtr.get(), &OCC::QuotaInfo::quotaUpdated,
                        this, &SynthesisPopover::onUpdateQuota);

                _accountStatusMap[accountId] = accountStatus;
                accountStatusIt = _accountStatusMap.find(accountId);
            }

            OCC::Folder::Map folderMap = OCC::FolderMan::instance()->map();
            for (auto folderIt = folderMap.begin(); folderIt != folderMap.end(); folderIt++) {
                OCC::AccountPtr folderAccount = folderIt.value()->accountState()->account();
                if (folderAccount->id() == accountId) {
                    auto folderInfoIt = accountStatusIt->second._folderMap.find(folderIt.key());
                    if (folderInfoIt == accountStatusIt->second._folderMap.end()) {
                        // New folder
                        FolderInfo folderInfo(folderIt.value()->shortGuiLocalPath(), folderIt.value()->path());
                        accountStatusIt->second._folderMap[folderIt.key()] = folderInfo;
                        folderInfoIt = accountStatusIt->second._folderMap.find(folderIt.key());
                    }

                    folderInfoIt->second._status = folderIt.value()->syncResult().status();
                }
            }

            // Manage removed folders
            for (auto folderInfoIt = accountStatusIt->second._folderMap.begin();
                 folderInfoIt != accountStatusIt->second._folderMap.end();
                 folderInfoIt++) {
                if (folderMap.find(folderInfoIt->first) == folderMap.end()) {
                    accountStatusIt->second._folderMap.erase(folderInfoIt);
                }
            }

            // Compute account status
            accountStatusIt->second._status = computeAccountStatus(accountStatusIt->second._folderMap);

            _driveSelectionWidget->addOrUpdateDrive(accountId,
                                                    accountStatePtr->account()->driveName(),
                                                    accountStatePtr->account()->getDriveColor(),
                                                    accountStatusIt->second._status);

            if (_currentAccountId.isEmpty() || !currentAccountStillExists) {
                _currentAccountId = accountId;
            }
        }

        // Manage removed accounts
        for (auto accountStatusIt = _accountStatusMap.begin(); accountStatusIt != _accountStatusMap.end(); accountStatusIt++) {
            if (!OCC::AccountManager::instance()->getAccountFromId(accountStatusIt->first)) {
                _accountStatusMap.erase(accountStatusIt);
            }
        }

        _driveSelectionWidget->selectDrive(_currentAccountId);
    }

    _folderButton->setEnabled(!_currentAccountId.isEmpty());
    _webviewButton->setEnabled(!_currentAccountId.isEmpty());
}

void SynthesisPopover::onUpdateProgress(const QString &folderId, const OCC::ProgressInfo &progress)
{
    std::cout << "onUpdateProgress folder: " << folderId.toStdString() << std::endl;

    OCC::Folder *folder = OCC::FolderMan::instance()->folder(folderId);
    if (folder) {
        OCC::AccountPtr account = folder->accountState()->account();
        if (account) {
            const auto accountStatusIt = _accountStatusMap.find(account->id());
            if (accountStatusIt != _accountStatusMap.end()) {
                const auto folderIt = accountStatusIt->second._folderMap.find(folderId);
                if (folderIt != accountStatusIt->second._folderMap.end()) {
                    folderIt->second._currentFile = progress.currentFile();
                    folderIt->second._totalFiles = qMax(progress.currentFile(), progress.totalFiles());
                    folderIt->second._completedSize = progress.completedSize();
                    folderIt->second._totalSize = qMax(progress.completedSize(), progress.totalSize());
                    folderIt->second._estimatedRemainingTime = progress.totalProgress().estimatedEta;
                    folderIt->second._syncPaused = folder->syncPaused();
                    folderIt->second._status = folder->syncResult().status();

                    // Compute account status
                    accountStatusIt->second._status = computeAccountStatus(_accountStatusMap[account->id()]._folderMap);

                    _driveSelectionWidget->addOrUpdateDrive(account->id(), account->driveName(), account->getDriveColor(),
                                                            accountStatusIt->second._status);

                    if (account->id() == _currentAccountId) {
                        // Compute account progress data
                        qint64 currentFile = 0;
                        qint64 totalFiles = 0;
                        qint64 completedSize = 0;
                        qint64 totalSize = 0;
                        qint64 estimatedRemainingTime = 0;
                        computeAccountProgress(accountStatusIt->second._folderMap, currentFile, totalFiles, completedSize, totalSize,
                                             estimatedRemainingTime);

                        _statusBarWidget->setStatus(accountStatusIt->second._status, currentFile, totalFiles,
                                                    estimatedRemainingTime);
                    }
                }
            }
        }
    }
}

void SynthesisPopover::onUpdateQuota(qint64 total, qint64 used)
{
    QString accountId = qvariant_cast<QString>(sender()->property(accountIdProperty));

    std::cout << "onUpdateQuota account: " << accountId.toStdString() << std::endl;

    const auto accountStatusIt = _accountStatusMap.find(accountId);
    if (accountStatusIt != _accountStatusMap.end()) {
        accountStatusIt->second._totalSize = total;
        accountStatusIt->second._used = used;

        if (accountId == _currentAccountId) {
            _progressBarWidget->setUsedSize(total, used);
        }
    }
}

void SynthesisPopover::onItemCompleted(const QString &folderId, const OCC::SyncFileItemPtr &syncFileItemPtr)
{
    if (syncFileItemPtr.data()->_status == OCC::SyncFileItem::FileIgnored) {
        return;
    }

    OCC::Folder *folder = OCC::FolderMan::instance()->folder(folderId);
    if (folder) {
        OCC::AccountPtr account = folder->accountState()->account();
        if (account) {
            const auto accountStatusIt = _accountStatusMap.find(account->id());
            if (accountStatusIt != _accountStatusMap.end() && accountStatusIt->second._synchronizedListWidget) {
                // Add item to synchronized list
                QListWidgetItem *item = new QListWidgetItem();
                SynchronizedItem synchronizedItem = SynchronizedItem(folderId, syncFileItemPtr.data()->_file,
                                                                     syncFileItemPtr.data()->_fileId,
                                                                     QDateTime::currentDateTime(),
                                                                     syncFileItemPtr.data()->_status,
                                                                     syncFileItemPtr.data()->_direction);
                accountStatusIt->second._synchronizedListWidget->insertItem(0, item);
                SynchronizedItemWidget *widget = new SynchronizedItemWidget(synchronizedItem,
                                                                            accountStatusIt->second._synchronizedListWidget);
                accountStatusIt->second._synchronizedListWidget->setItemWidget(item, widget);
                connect(widget, &SynchronizedItemWidget::openFolder, this, &SynthesisPopover::onOpenFolderItem);
                connect(widget, &SynchronizedItemWidget::open, this, &SynthesisPopover::onOpenItem);
                connect(widget, &SynchronizedItemWidget::addToFavourites, this, &SynthesisPopover::onAddToFavouriteItem);
                connect(widget, &SynchronizedItemWidget::manageRightAndSharing, this, &SynthesisPopover::onManageRightAndSharingItem);
                connect(widget, &SynchronizedItemWidget::copyLink, this, &SynthesisPopover::onCopyLinkItem);
                connect(widget, &SynchronizedItemWidget::displayOnWebview, this, &SynthesisPopover::onOpenWebviewItem);

                // Scroll to current item
                if (accountStatusIt->second._currentSynchronizedWidgetItem) {
                    accountStatusIt->second._synchronizedListWidget->scrollToItem(
                                accountStatusIt->second._currentSynchronizedWidgetItem);
                }

                if (accountStatusIt->second._synchronizedListWidget->count() > maxSynchronizedItems) {
                    // Remove last row
                    QListWidgetItem *lastWidgetItem = accountStatusIt->second._synchronizedListWidget->takeItem(
                                accountStatusIt->second._synchronizedListWidget->count() - 1);
                    if (lastWidgetItem == accountStatusIt->second._currentSynchronizedWidgetItem) {
                        accountStatusIt->second._currentSynchronizedWidgetItem = nullptr;
                    }
                    delete lastWidgetItem;
                }
            }
        }
    }
}

void SynthesisPopover::onOpenFolderMenu(bool checked)
{
    Q_UNUSED(checked)

    auto accountStatusIt = _accountStatusMap.find(_currentAccountId);
    if (accountStatusIt != _accountStatusMap.end()) {
        if (accountStatusIt->second._folderMap.size() == 1) {
            // Open folder
            auto folderInfoIt = accountStatusIt->second._folderMap.begin();
            openUrl(folderInfoIt->first);
        }
        else if (accountStatusIt->second._folderMap.size() > 1) {
            // Open menu
            MenuWidget *menu = new MenuWidget(this);
            for (auto folderInfoIt : accountStatusIt->second._folderMap) {
                QWidgetAction *openFolderAction = new QWidgetAction(this);
                openFolderAction->setProperty(MenuWidget::actionTypeProperty.c_str(), folderInfoIt.first);
                MenuItemWidget *openFolderMenuItemWidget = new MenuItemWidget(folderInfoIt.second._name);
                openFolderMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/folder.svg");
                openFolderAction->setDefaultWidget(openFolderMenuItemWidget);
                connect(openFolderAction, &QWidgetAction::triggered, this, &SynthesisPopover::onOpenFolder);
                menu->addAction(openFolderAction);
            }
            menu->exec(QWidget::mapToGlobal(_folderButton->geometry().center()), true);
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
            qCWarning(lcSynthesisPopover) << "QDesktopServices::openUrl failed for " << accountPtr->url();
        }
    }
}

void SynthesisPopover::onOpenMenu(bool checked)
{
    Q_UNUSED(checked)

    if (_menuButton) {
        MenuWidget *menu = new MenuWidget(this);

        // Parameters
        QWidgetAction *parametersAction = new QWidgetAction(this);
        MenuItemWidget *parametersMenuItemWidget = new MenuItemWidget(tr("Parameters"));
        parametersMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/parameters.svg");
        parametersAction->setDefaultWidget(parametersMenuItemWidget);
        connect(parametersAction, &QWidgetAction::triggered, this, &SynthesisPopover::onOpenParameters);
        menu->addAction(parametersAction);

        // Disable Notifications
        QWidgetAction *notificationsAction = new QWidgetAction(this);
        MenuItemWidget *notificationsMenuItemWidget = new MenuItemWidget(tr("Disable Notifications"));
        notificationsMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/notification-off.svg");
        notificationsMenuItemWidget->setHasSubmenu(true);
        notificationsAction->setDefaultWidget(notificationsMenuItemWidget);
        menu->addAction(notificationsAction);

        // Disable Notifications submenu
        static const QVector<QString> notificationActionsText =
            QVector<QString>()
            << QString(tr("Never"))
            << QString(tr("During 1 hour"))
            << QString(tr("Until tomorrow"))
            << QString(tr("During 3 days"))
            << QString(tr("During 1 week"))
            << QString(tr("Always"));

        MenuWidget *submenu = new MenuWidget(this);

        QActionGroup *notificationActionGroup = new QActionGroup(this);
        notificationActionGroup->setExclusive(true);

        QWidgetAction *notificationAction;
        for (int i = 0; i < notificationActionsText.size(); i++) {
            notificationAction = new QWidgetAction(this);
            notificationAction->setProperty(MenuWidget::actionTypeProperty.c_str(), i);
            MenuItemWidget *notificationMenuItemWidget = new MenuItemWidget(notificationActionsText[i]);
            notificationMenuItemWidget->setChecked(i == _notificationAction);
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

        menu->exec(QWidget::mapToGlobal(_menuButton->geometry().center()), true);
    }
}

void SynthesisPopover::onOpenParameters(bool checked)
{
    Q_UNUSED(checked)

    emit openParametersDialog();
}

void SynthesisPopover::onDisplayHelp(bool checked)
{
    Q_UNUSED(checked)

    emit openHelp();
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

    _notificationAction = qvariant_cast<notificationActions>(sender()->property(MenuWidget::actionTypeProperty.c_str()));
}

void SynthesisPopover::onAccountSelected(QString id)
{
    std::cout << "onAccountSelected account: " << id.toStdString() << std::endl;

    const auto accountStatusIt = _accountStatusMap.find(id);
    if (accountStatusIt != _accountStatusMap.end()) {
        _currentAccountId = id;

        _progressBarWidget->setUsedSize(accountStatusIt->second._totalSize, accountStatusIt->second._used);

        // Compute account progress data
        qint64 currentFile = 0;
        qint64 totalFiles = 0;
        qint64 completedSize = 0;
        qint64 totalSize = 0;
        qint64 estimatedRemainingTime = 0;
        computeAccountProgress(_accountStatusMap[id]._folderMap,
                             currentFile, totalFiles,
                             completedSize, totalSize,
                             estimatedRemainingTime);

        _statusBarWidget->setStatus(accountStatusIt->second._status, currentFile, totalFiles, estimatedRemainingTime);

        _stackedWidget->setCurrentIndex(accountStatusIt->second._synchronizedListStackPosition);
    }
}

void SynthesisPopover::onPauseSync()
{
    pauseSync(true);
}

void SynthesisPopover::onResumeSync()
{
    pauseSync(false);
}

void SynthesisPopover::onRunSync()
{
    // Terminate and reschedule any running sync
    OCC::FolderMan *folderMan = OCC::FolderMan::instance();
    for (auto folder : folderMan->map()) {
        if (folder->isSyncRunning()) {
            folder->slotTerminateSync();
            folderMan->scheduleFolder(folder);
        }
    }

    for (auto folder : folderMan->map()) {
        OCC::AccountPtr account = folder->accountState()->account();
        if (account) {
            if (_currentAccountId == account->id()) {
                folder->slotWipeErrorBlacklist();

                // Insert the selected folder at the front of the queue
                folderMan->scheduleFolderNext(folder);
            }
        }
    }
}

void SynthesisPopover::onButtonBarToggled(int position)
{
    const auto accountStatusIt = _accountStatusMap.find(_currentAccountId);
    if (accountStatusIt != _accountStatusMap.end()) {
        switch (position) {
        case stackedWidget::Synchronized:
            if (accountStatusIt->second._synchronizedListStackPosition) {
                _stackedWidget->setCurrentIndex(accountStatusIt->second._synchronizedListStackPosition);
            }
            else {
                _stackedWidget->setCurrentIndex(stackedWidget::Synchronized);
            }
            break;
        case stackedWidget::Favorites:
            if (accountStatusIt->second._favoritesListStackPosition) {
                _stackedWidget->setCurrentIndex(accountStatusIt->second._favoritesListStackPosition);
            }
            else {
                _stackedWidget->setCurrentIndex(stackedWidget::Favorites);
            }
            break;
        case stackedWidget::Activity:
            if (accountStatusIt->second._activityListStackPosition) {
                _stackedWidget->setCurrentIndex(accountStatusIt->second._activityListStackPosition);
            }
            else {
                _stackedWidget->setCurrentIndex(stackedWidget::Activity);
            }
            break;
        }
    }
}

void SynthesisPopover::onCurrentSynchronizedWidgetItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    const auto accountStatusIt = _accountStatusMap.find(_currentAccountId);
    if (accountStatusIt != _accountStatusMap.end()) {
        if (previous) {
            SynchronizedItemWidget *previousWidget =
                    dynamic_cast<SynchronizedItemWidget *>(accountStatusIt->second._synchronizedListWidget->itemWidget(previous));
            previousWidget->setSelected(false);
        }

        if (current) {
            SynchronizedItemWidget *currentWidget =
                    dynamic_cast<SynchronizedItemWidget *>(accountStatusIt->second._synchronizedListWidget->itemWidget(current));
            currentWidget->setSelected(true);
            accountStatusIt->second._currentSynchronizedWidgetItem = current;
        }
    }
}

void SynthesisPopover::onOpenFolderItem()
{
    const SynchronizedItem *synchronizedItem = currentSynchronizedItem();
    if (synchronizedItem) {
        QString fullFilePath = folderPath(synchronizedItem->folderId(), synchronizedItem->filePath());
        if (!fullFilePath.isEmpty()) {
            OCC::showInFileManager(fullFilePath);
        }
    }
}

void SynthesisPopover::onOpenItem()
{
    const SynchronizedItem *synchronizedItem = currentSynchronizedItem();
    if (synchronizedItem) {
        openUrl(synchronizedItem->folderId(), synchronizedItem->filePath());
    }
}

void SynthesisPopover::onAddToFavouriteItem()
{

}

void SynthesisPopover::onManageRightAndSharingItem()
{
    const SynchronizedItem *synchronizedItem = currentSynchronizedItem();
    if (synchronizedItem) {
        QString folderRelativePath;
        QString fullFilePath = folderPath(synchronizedItem->folderId(), synchronizedItem->filePath());
        OCC::FolderMan::instance()->folderForPath(fullFilePath, &folderRelativePath);
        if (folderRelativePath == "/") {
            qCDebug(lcSynthesisPopover) << "Cannot share root directory!";
        }
        else {
            emit openShareDialogPublicLinks(folderRelativePath, fullFilePath);
        }
    }
}

void SynthesisPopover::onCopyLinkItem()
{
    const SynchronizedItem *synchronizedItem = currentSynchronizedItem();
    if (synchronizedItem) {
        QString folderRelativePath;
        QString fullFilePath = folderPath(synchronizedItem->folderId(), synchronizedItem->filePath());
        OCC::FolderMan::instance()->folderForPath(fullFilePath, &folderRelativePath);

        OCC::Folder *folder = OCC::FolderMan::instance()->folder(synchronizedItem->folderId());
        QString serverRelativePath = QDir(folder->remotePath()).filePath(folderRelativePath);
        OCC::AccountPtr accountPtr = OCC::AccountManager::instance()->getAccountFromId(_currentAccountId);
        auto job = new OCC::GetOrCreatePublicLinkShare(accountPtr, serverRelativePath, this);
        connect(job, &OCC::GetOrCreatePublicLinkShare::done, this,
                [](const QString &url) { QApplication::clipboard()->setText(url); });
        connect(job, &OCC::GetOrCreatePublicLinkShare::error, this,
                [=]() { /*emit shareCommandReceived(serverRelativePath, fullFilePath, OCC::ShareDialogStartPage::PublicLinks);*/ });

        job->run();
    }
}

void SynthesisPopover::onOpenWebviewItem()
{
    const SynchronizedItem *synchronizedItem = currentSynchronizedItem();
    if (synchronizedItem) {
        OCC::AccountPtr accountPtr = OCC::AccountManager::instance()->getAccountFromId(_currentAccountId);
        OCC::fetchPrivateLinkUrl(accountPtr, synchronizedItem->filePath(), synchronizedItem->fileId(), this,
            [this](const QString &url) {
                OCC::Utility::openBrowser(url, this);
            });
    }
}

SynthesisPopover::FolderInfo::FolderInfo(const QString &name, const QString &path)
    : _name(name)
    , _path(path)
    , _currentFile(0)
    , _totalFiles(0)
    , _completedSize(0)
    , _totalSize(0)
    , _estimatedRemainingTime(0)
    , _syncPaused(false)
    , _status(OCC::SyncResult::Status::Undefined)
{
}

SynthesisPopover::AccountStatus::AccountStatus(OCC::AccountState *accountState)
    : _totalSize(0)
    , _used(0)
    , _status(OCC::SyncResult::Status::Undefined)
    , _folderMap(std::map<QString, FolderInfo>())
    , _synchronizedListWidget(nullptr)
    , _currentSynchronizedWidgetItem(nullptr)
    , _synchronizedListStackPosition(stackedWidget::Synchronized)
    , _favoritesListStackPosition(stackedWidget::Favorites)
    , _activityListStackPosition(stackedWidget::Activity)
{
    if (accountState) {
        _quotaInfoPtr = std::unique_ptr<OCC::QuotaInfo>(new OCC::QuotaInfo(accountState));
        _quotaInfoPtr.get()->setActive(true);
        _quotaInfoPtr.get()->setProperty(accountIdProperty, accountState->account()->id());
    }
}

}
