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
#include "customtogglepushbutton.h"
#include "synchronizeditem.h"
#include "errorspopup.h"
#include "custommessagebox.h"
#include "accountmanager.h"
#include "folderman.h"
#include "account.h"
#include "common/utility.h"
#include "guiutility.h"
#include "openfilemanager.h"
#include "getorcreatepubliclinkshare.h"
#include "configfile.h"
#include "theme.h"
#include "owncloudgui.h"

#undef CONSOLE_DEBUG
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
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QWidgetAction>

namespace KDC {

static const QSize windowSize(440, 575);
static const int triangleHeight = 10;
static const int triangleWidth  = 20;
static const int trianglePosition = 100; // Position from side
static const int cornerRadius = 5;
static const int shadowBlurRadius = 20;
static const int toolBarHMargin = 10;
static const int toolBarVMargin = 10;
static const int toolBarSpacing = 10;
static const int driveBoxHMargin = 10;
static const int driveBoxVMargin = 10;
static const int defaultPageSpacing = 20;
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

Q_LOGGING_CATEGORY(lcSynthesisPopover, "gui.synthesispopover", QtInfoMsg)

SynthesisPopover::SynthesisPopover(bool debugMode, OCC::OwnCloudGui *gui, QWidget *parent)
    : QDialog(parent)
    , _debugMode(debugMode)
    , _gui(gui)
    , _sysTrayIconRect(QRect())
    , _currentAccountId(QString())
    , _backgroundMainColor(QColor())
    , _errorsButton(nullptr)
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
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint
                   | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

    setMinimumSize(windowSize);
    setMaximumSize(windowSize);

    OCC::ConfigFile cfg;
    _notificationsDisabled = cfg.optionalDesktopNotifications()
            ? NotificationsDisabled::Never
            : NotificationsDisabled::Always;

    initUI();
    onRefreshAccountList();

    connect(OCC::FolderMan::instance(), &OCC::FolderMan::folderSyncStateChange,
            this, &SynthesisPopover::onRefreshAccountList);
    connect(OCC::FolderMan::instance(), &OCC::FolderMan::folderListChanged,
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

    QScreen *screen = QGuiApplication::screenAt(_sysTrayIconRect.center());
    if (!screen) {
        return;
    }

    QRect screenRect = screen->availableGeometry();
    OCC::Utility::systrayPosition position = OCC::Utility::getSystrayPosition(screen);

    bool trianglePositionLeft;
    bool trianglePositionTop;
    if (_sysTrayIconRect != QRect()
            && OCC::Utility::isPointInSystray(screen, _sysTrayIconRect.center())) {
        if (position == OCC::Utility::systrayPosition::Top || position == OCC::Utility::systrayPosition::Bottom) {
            // Triangle position (left/right)
            trianglePositionLeft =
                    (_sysTrayIconRect.center().x() + rect().width() - trianglePosition
                    < screenRect.x() + screenRect.width());
        }
        else if (position == OCC::Utility::systrayPosition::Left || position == OCC::Utility::systrayPosition::Right) {
            // Triangle position (top/bottom)
            trianglePositionTop =
                    (_sysTrayIconRect.center().y() + rect().height() - trianglePosition
                    < screenRect.y() + screenRect.height());
        }
    }

    // Move window
    QPoint popoverPosition;
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
    }
    else {
        if (position == OCC::Utility::systrayPosition::Top) {
            // Dialog position
            popoverPosition = QPoint(
                trianglePositionLeft
                ? _sysTrayIconRect.center().x() - trianglePosition
                : _sysTrayIconRect.center().x() - rect().width() + trianglePosition,
                screenRect.y());
        }
        else if (position == OCC::Utility::systrayPosition::Bottom) {
            // Dialog position
            popoverPosition = QPoint(
                trianglePositionLeft
                ? _sysTrayIconRect.center().x() - trianglePosition
                : _sysTrayIconRect.center().x() - rect().width() + trianglePosition,
                screenRect.y() + screenRect.height() - rect().height());
        }
        else if (position == OCC::Utility::systrayPosition::Left) {
            // Dialog position
            popoverPosition = QPoint(
                screenRect.x(),
                trianglePositionTop
                ? _sysTrayIconRect.center().y() - trianglePosition
                : _sysTrayIconRect.center().y() - rect().height() + trianglePosition);
        }
        else if (position == OCC::Utility::systrayPosition::Right) {
            // Dialog position
            popoverPosition = QPoint(
                screenRect.x() + screenRect.width() - rect().width(),
                trianglePositionTop
                ? _sysTrayIconRect.center().y() - trianglePosition
                : _sysTrayIconRect.center().y() - rect().height() + trianglePosition);
        }
    }

    move(popoverPosition);
}

void SynthesisPopover::forceRedraw()
{
#ifdef Q_OS_WINDOWS
    // Windows hack
    QTimer::singleShot(0, this, [=]()
    {
        if (geometry().height() > windowSize.height()) {
            setGeometry(geometry() + QMargins(0, 0, 0, -1));
            setMaximumHeight(windowSize.height());
        }
        else {
            setMaximumHeight(windowSize.height() + 1);
            setGeometry(geometry() + QMargins(0, 0, 0, 1));
        }
    });
#endif
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

    QRect screenRect = screen->availableGeometry();
    OCC::Utility::systrayPosition position = OCC::Utility::getSystrayPosition(screen);

    bool trianglePositionLeft;
    bool trianglePositionTop;
    if (_sysTrayIconRect != QRect()
            && OCC::Utility::isPointInSystray(screen, _sysTrayIconRect.center())) {
        if (position == OCC::Utility::systrayPosition::Top || position == OCC::Utility::systrayPosition::Bottom) {
            // Triangle position (left/right)
            trianglePositionLeft =
                    (_sysTrayIconRect.center().x() + rect().width() - trianglePosition
                    < screenRect.x() + screenRect.width());
        }
        else if (position == OCC::Utility::systrayPosition::Left || position == OCC::Utility::systrayPosition::Right) {
            // Triangle position (top/bottom)
            trianglePositionTop =
                    (_sysTrayIconRect.center().y() + rect().height() - trianglePosition
                    < screenRect.y() + screenRect.height());
        }
    }

    // Calculate border path
    QPainterPath painterPath;
    QRect intRect = rect().marginsRemoved(QMargins(triangleHeight, triangleHeight, triangleHeight - 1, triangleHeight - 1));
    if (_sysTrayIconRect == QRect()
            || !OCC::Utility::isPointInSystray(screen, _sysTrayIconRect.center())) {
        // Border
        painterPath.addRoundedRect(intRect, cornerRadius, cornerRadius);
    }
    else {
        int cornerDiameter = 2 * cornerRadius;
        QPointF trianglePoint1;
        QPointF trianglePoint2;
        QPointF trianglePoint3;
        if (position == OCC::Utility::systrayPosition::Top) {
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

    // Update shadow color
    QGraphicsDropShadowEffect *effect = qobject_cast<QGraphicsDropShadowEffect *>(graphicsEffect());
    if (effect) {
        effect->setColor(OCC::Utility::getShadowColor(true));
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(backgroundMainColor());
    painter.setPen(Qt::NoPen);
    painter.drawPath(painterPath);
}

bool SynthesisPopover::event(QEvent *event)
{
    bool ret = QDialog::event(event);
    if (event->type() == QEvent::WindowDeactivate) {
        setGraphicsEffect(nullptr);
        done(QDialog::Accepted);
    }
    else if (event->type() == QEvent::Show || event->type() == QEvent::Hide) {
        // Activate/deactivate quota request
        auto accountInfoIt = _accountInfoMap.begin();
        while (accountInfoIt != _accountInfoMap.end()) {
            accountInfoIt->second._quotaInfoPtr->setActive(event->type() == QEvent::Show);
            accountInfoIt++;
        }
    }
    return ret;
}

void SynthesisPopover::initUI()
{
    /*
     *  mainVBox
     *      hBoxToolBar
     *          iconLabel
     *          _errorsButton
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

    _errorsButton = new CustomToolButton(this);
    _errorsButton->setObjectName("errorsButton");
    _errorsButton->setIconPath(":/client/resources/icons/actions/warning.svg");
    _errorsButton->setToolTip(tr("Show warnings and errors"));
    _errorsButton->setVisible(false);
    _errorsButton->setWithMenu(true);
    hBoxToolBar->addWidget(_errorsButton);

    _folderButton = new CustomToolButton(this);
    _folderButton->setIconPath(":/client/resources/icons/actions/folder.svg");
    _folderButton->setToolTip(tr("Show in folder"));
    _folderButton->setVisible(false);
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
    hBoxDriveBar->setContentsMargins(driveBoxHMargin, driveBoxVMargin, driveBoxHMargin, driveBoxVMargin);

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
    _buttonsBarWidget->hide();
    mainVBox->addWidget(_buttonsBarWidget);

    CustomTogglePushButton *synchronizedButton = new CustomTogglePushButton(tr("Synchronized"), _buttonsBarWidget);
    synchronizedButton->setIconPath(":/client/resources/icons/actions/sync.svg");
    _buttonsBarWidget->insertButton(StackedWidget::Synchronized, synchronizedButton);

    CustomTogglePushButton *favoritesButton = new CustomTogglePushButton(tr("Favorites"), _buttonsBarWidget);
    favoritesButton->setIconPath(":/client/resources/icons/actions/favorite.svg");
    _buttonsBarWidget->insertButton(StackedWidget::Favorites, favoritesButton);

    CustomTogglePushButton *activityButton = new CustomTogglePushButton(tr("Activity"), _buttonsBarWidget);
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
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(this);
    effect->setBlurRadius(shadowBlurRadius);
    effect->setOffset(0);
    effect->setColor(OCC::Utility::getShadowColor(true));
    setGraphicsEffect(effect);

    connect(_errorsButton, &CustomToolButton::clicked, this, &SynthesisPopover::onOpenErrorsMenu);
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
    QString fullFilePath = folderPath(folderId, filePath);
    return OCC::Utility::getUrlFromLocalPath(fullFilePath);
}

QString SynthesisPopover::folderPath(const QString &folderId, const QString &filePath)
{
    for (auto const &accountInfoMapElt : _accountInfoMap) {
        if (accountInfoMapElt.second._folderMap.find(folderId) != accountInfoMapElt.second._folderMap.end()) {
            return accountInfoMapElt.second.folderPath(folderId, filePath);
        }
    }

    return QString();
}

void SynthesisPopover::openUrl(const QString &folderId, const QString &filePath)
{
    if (!folderId.isEmpty()) {
        QUrl url = folderUrl(folderId, filePath);
        if (url.isValid()) {
            if (!QDesktopServices::openUrl(url)) {
                CustomMessageBox *msgBox = new CustomMessageBox(
                            QMessageBox::Warning,
                            tr("Unable to open folder url %1.").arg(url.toString()),
                            QMessageBox::Ok, this);
                msgBox->exec();
            }
        }
    }
}

const FolderInfo *SynthesisPopover::getFirstFolderWithStatus(const FolderMap &folderMap,
                                                             OCC::SyncResult::Status status)
{
    const FolderInfo *folderInfo = nullptr;
    for (auto const &folderMapElt : folderMap) {
        if (folderMapElt.second.get()) {
            if (folderMapElt.second->_status == status) {
                folderInfo = folderMapElt.second.get();
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

const FolderInfo *SynthesisPopover::getFirstFolderByPriority(const FolderMap &folderMap)
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
        folderInfo = (folderMap.empty() ? nullptr : folderMap.begin()->second.get());
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
        _statusBarWidget->setCurrentAccount(&accountInfoIt->second);
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
        vboxLayout->setSpacing(defaultPageSpacing);
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
        defaultTextLabel->setContextMenuPolicy(Qt::PreventContextMenu);

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

    if (OCC::AccountManager::instance()->accounts().isEmpty() && _accountInfoMap.size() == 0) {
        reset();
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
                                    accountInfoIt->second._folderMap[folderIt.key()] = std::shared_ptr<FolderInfo>(
                                            new FolderInfo(folderIt.value()->shortGuiLocalPath(), folderIt.value()->path()));
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

                if (accountInfoIt->second._folderMap.size() == 0) {
                    _statusBarWidget->reset();
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
                if (accountInfoIt->second._synchronizedListStackPosition) {
                    _stackedWidget->removeWidget(_stackedWidget->widget(accountInfoIt->second._synchronizedListStackPosition));
                }
                _driveSelectionWidget->removeDrive(accountInfoIt->first);
                accountInfoIt = _accountInfoMap.erase(accountInfoIt);
            }
            else {
                accountInfoIt++;
            }
        }

        if (_accountInfoMap.size() > 0) {
            // Count drives with warning/errors
            bool drivesWithErrors = false;
            for (auto const &accountInfo : _accountInfoMap) {
                if (_gui->driveErrorCount(accountInfo.first) > 0) {
                    drivesWithErrors = true;
                    break;
                }
            }

            _errorsButton->setVisible(drivesWithErrors);
            _folderButton->setVisible(currentFolderMapSize > 0);
            _folderButton->setWithMenu(currentFolderMapSize > 1);
            _webviewButton->setVisible(currentFolderMapSize > 0);
            _statusBarWidget->setSeveralDrives(_accountInfoMap.size() > 1);
            _driveSelectionWidget->selectDrive(_currentAccountId);
            refreshStatusBar(_currentAccountId);
        }
        else {
            reset();
        }
    }

    setSynchronizedDefaultPage(&_defaultSynchronizedPageWidget, this);
    forceRedraw();
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
                            qCDebug(lcSynthesisPopover) << "Null pointer!";
                            Q_ASSERT(false);
                        }

                        if (account->id() == _currentAccountId) {
                            refreshStatusBar(folderInfoIt->second.get());
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
        OCC::Folder *folder = OCC::FolderMan::instance()->folder(folderId);
        if (folder) {
            if (folder->accountState()) {
                OCC::AccountPtr account = folder->accountState()->account();
                if (!account.isNull()) {
                    const auto accountInfoIt = _accountInfoMap.find(account->id());
                    if (accountInfoIt != _accountInfoMap.end()) {
                        if (item.data()->_status == OCC::SyncFileItem::NoStatus
                                || item.data()->_status == OCC::SyncFileItem::FatalError
                                || item.data()->_status == OCC::SyncFileItem::NormalError
                                || item.data()->_status == OCC::SyncFileItem::SoftError
                                || item.data()->_status == OCC::SyncFileItem::DetailError
                                || item.data()->_status == OCC::SyncFileItem::BlacklistedError
                                || item.data()->_status == OCC::SyncFileItem::FileIgnored) {
                            return;
                        }

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
                                                          item.data()->_type,
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
                        connect(widget, &SynchronizedItemWidget::selectionChanged, this, &SynthesisPopover::onSelectionChanged);
                        connect(this, &SynthesisPopover::cannotSelect, widget, &SynchronizedItemWidget::onCannotSelect);

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

void SynthesisPopover::onOpenErrorsMenu(bool checked)
{
    Q_UNUSED(checked)

    QList<ErrorsPopup::DriveError> driveErrorList = QList<ErrorsPopup::DriveError>();
    for (auto const &accountInfo : _accountInfoMap) {
        if (_gui->driveErrorCount(accountInfo.first) > 0) {
            ErrorsPopup::DriveError driveError;
            driveError.accountId = accountInfo.first;
            driveError.accountName = accountInfo.second._name;
            driveError.errorsCount = _gui->driveErrorCount(accountInfo.first);
            driveErrorList << driveError;
        }
    }

    if (driveErrorList.size() > 0) {
        QPoint position = QWidget::mapToGlobal(_errorsButton->geometry().center());
        ErrorsPopup *errorsPopup = new ErrorsPopup(driveErrorList, position, this);
        connect(errorsPopup, &ErrorsPopup::accountSelected, this, &SynthesisPopover::onDisplayErrors);
        errorsPopup->show();
        errorsPopup->setModal(true);
    }
}

void SynthesisPopover::onDisplayErrors(const QString &accountId)
{
    displayErrors(accountId);
}

void SynthesisPopover::displayErrors(const QString &accountId)
{
    emit openParametersDialog(accountId, true);
}

void SynthesisPopover::reset()
{
    _currentAccountId.clear();
    _accountInfoMap.clear();
    _errorsButton->setVisible(false);
    _folderButton->setVisible(false);
    _folderButton->setWithMenu(false);
    _webviewButton->setVisible(false);
    _driveSelectionWidget->clear();
    _progressBarWidget->reset();
    _statusBarWidget->reset();
    _stackedWidget->setCurrentIndex(StackedWidget::Synchronized);
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
            for (auto const &folderMapElt : accountInfoIt->second._folderMap) {
                QWidgetAction *openFolderAction = new QWidgetAction(this);
                openFolderAction->setProperty(MenuWidget::actionTypeProperty.c_str(), folderMapElt.first);
                MenuItemWidget *openFolderMenuItemWidget = new MenuItemWidget(folderMapElt.second->_name);
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
            CustomMessageBox *msgBox = new CustomMessageBox(
                        QMessageBox::Warning,
                        tr("Unable to access web site %1.").arg(accountPtr->url().toString()),
                        QMessageBox::Ok, this);
            msgBox->exec();
        }
    }
}

void SynthesisPopover::onOpenMiscellaneousMenu(bool checked)
{
    Q_UNUSED(checked)

    MenuWidget *menu = new MenuWidget(MenuWidget::Menu, this);

    // Drive parameters
    if (_accountInfoMap.size()) {
        QWidgetAction *driveParametersAction = new QWidgetAction(this);
        MenuItemWidget *driveParametersMenuItemWidget = new MenuItemWidget(tr("Drive parameters"));
        driveParametersMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/drive.svg");
        driveParametersAction->setDefaultWidget(driveParametersMenuItemWidget);
        connect(driveParametersAction, &QWidgetAction::triggered, this, &SynthesisPopover::onOpenDriveParameters);
        menu->addAction(driveParametersAction);
    }

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

    // Application preferences
    QWidgetAction *preferencesAction = new QWidgetAction(this);
    MenuItemWidget *preferencesMenuItemWidget = new MenuItemWidget(tr("Application preferences"));
    preferencesMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/parameters.svg");
    preferencesAction->setDefaultWidget(preferencesMenuItemWidget);
    connect(preferencesAction, &QWidgetAction::triggered, this, &SynthesisPopover::onOpenPreferences);
    menu->addAction(preferencesAction);

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
    for (auto const &notificationMapElt : notificationMap) {
        notificationAction = new QWidgetAction(this);
        notificationAction->setProperty(MenuWidget::actionTypeProperty.c_str(), notificationMapElt.first);
        QString text = QCoreApplication::translate("KDC::SynthesisPopover", notificationMapElt.second.toStdString().c_str());
        MenuItemWidget *notificationMenuItemWidget = new MenuItemWidget(text);
        notificationMenuItemWidget->setChecked(notificationMapElt.first == _notificationsDisabled);
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

void SynthesisPopover::onOpenPreferences(bool checked)
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

    hide();
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

    bool notificationAlreadyDisabledForPeriod = _notificationsDisabled != NotificationsDisabled::Never
            && _notificationsDisabled != NotificationsDisabled::Always;

    _notificationsDisabled = qvariant_cast<NotificationsDisabled>(sender()->property(MenuWidget::actionTypeProperty.c_str()));
    switch (_notificationsDisabled) {
    case NotificationsDisabled::Never:
        _notificationsDisabledUntilDateTime = QDateTime();
        break;
    case NotificationsDisabled::OneHour:
        _notificationsDisabledUntilDateTime = notificationAlreadyDisabledForPeriod
                ? _notificationsDisabledUntilDateTime.addSecs(60 * 60)
                : QDateTime::currentDateTime().addSecs(60 * 60);
        break;
    case NotificationsDisabled::UntilTomorrow:
        _notificationsDisabledUntilDateTime = QDateTime(QDateTime::currentDateTime().addDays(1).date(), QTime(8, 0));
        break;
    case NotificationsDisabled::TreeDays:
        _notificationsDisabledUntilDateTime = notificationAlreadyDisabledForPeriod
                ? _notificationsDisabledUntilDateTime.addDays(3)
                : QDateTime::currentDateTime().addDays(3);
        break;
    case NotificationsDisabled::OneWeek:
        _notificationsDisabledUntilDateTime = notificationAlreadyDisabledForPeriod
                ? _notificationsDisabledUntilDateTime.addDays(7)
                : QDateTime::currentDateTime().addDays(7);
        break;
    case NotificationsDisabled::Always:
        _notificationsDisabledUntilDateTime = QDateTime();
        break;
    }

    emit disableNotifications(_notificationsDisabled, _notificationsDisabledUntilDateTime);
}

void SynthesisPopover::onOpenDriveParameters(bool checked)
{
    Q_UNUSED(checked)

    emit openParametersDialog(_currentAccountId);
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
        _buttonsBarWidget->selectButton(int(accountInfoIt->second._stackedWidget));
    }
}

void SynthesisPopover::onAddDrive()
{
    emit addDrive();
}

void SynthesisPopover::onPauseSync(StatusBarWidget::ActionType type, const QString &id)
{
    OCC::Utility::pauseSync(
                type == StatusBarWidget::ActionType::Drive || type == StatusBarWidget::ActionType::Folder
                ? _currentAccountId
                : QString(),
                type == StatusBarWidget::ActionType::Folder ? id : QString(),
                true);
}

void SynthesisPopover::onResumeSync(StatusBarWidget::ActionType type, const QString &id)
{
    OCC::Utility::pauseSync(
                type == StatusBarWidget::ActionType::Drive || type == StatusBarWidget::ActionType::Folder
                ? _currentAccountId
                : QString(),
                type == StatusBarWidget::ActionType::Folder ? id : QString(),
                false);
}

void SynthesisPopover::onRunSync(StatusBarWidget::ActionType type, const QString &id)
{
    OCC::Utility::runSync(
                type == StatusBarWidget::ActionType::Drive || type == StatusBarWidget::ActionType::Folder
                ? _currentAccountId
                : QString(),
                type == StatusBarWidget::ActionType::Folder ? id : QString());
}

void SynthesisPopover::onButtonBarToggled(int position)
{
    const auto accountInfoIt = _accountInfoMap.find(_currentAccountId);
    if (accountInfoIt != _accountInfoMap.end()) {
        accountInfoIt->second._stackedWidget = StackedWidget(position);
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

    CustomMessageBox *msgBox = new CustomMessageBox(
                QMessageBox::Information,
                tr("Not implemented!"),
                QMessageBox::Ok, this);
    msgBox->exec();
}

void SynthesisPopover::onManageRightAndSharingItem(const SynchronizedItem &item)
{
    QString folderRelativePath;
    QString fullFilePath = folderPath(item.folderId(), item.filePath());
    OCC::FolderMan::instance()->folderForPath(fullFilePath, &folderRelativePath);
    if (folderRelativePath == dirSeparator) {
        qCDebug(lcSynthesisPopover) << "Cannot share root directory!";
        CustomMessageBox *msgBox = new CustomMessageBox(
                    QMessageBox::Information,
                    tr("You cannot share the root directory of your Drive!"),
                    QMessageBox::Ok, this);
        msgBox->exec();
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

void SynthesisPopover::onSelectionChanged(bool isSelected)
{
    emit cannotSelect(isSelected);
    forceRedraw();
}

void SynthesisPopover::onCopyUrlToClipboard(const QString &url)
{
    QApplication::clipboard()->setText(url);
    CustomMessageBox *msgBox = new CustomMessageBox(
                QMessageBox::Information,
                tr("The shared link has been copied to the clipboard."),
                QMessageBox::Ok, this);
    msgBox->exec();
}

void SynthesisPopover::onLinkActivated(const QString &link)
{
    if (link == OCC::Utility::learnMoreLink) {
        displayErrors(_currentAccountId);
    }
    else {
        // URL link
        QUrl url = QUrl(link);
        if (url.isValid()) {
            if (!QDesktopServices::openUrl(url)) {
                qCWarning(lcSynthesisPopover) << "QDesktopServices::openUrl failed for " << link;
                CustomMessageBox *msgBox = new CustomMessageBox(
                            QMessageBox::Warning,
                            tr("Unable to open link %1.").arg(link),
                            QMessageBox::Ok, this);
                msgBox->exec();
            }
        }
        else {
            qCWarning(lcSynthesisPopover) << "Invalid link " << link;
            CustomMessageBox *msgBox = new CustomMessageBox(
                        QMessageBox::Warning,
                        tr("Invalid link %1.").arg(link),
                        QMessageBox::Ok, this);
            msgBox->exec();
        }
    }
}

SynthesisPopover::AccountInfoSynthesis::AccountInfoSynthesis()
    : AccountInfo()
    , _stackedWidget(StackedWidget::Synchronized)
    , _synchronizedListWidget(nullptr)
    , _synchronizedListStackPosition(StackedWidget::Synchronized)
    , _favoritesListStackPosition(StackedWidget::Favorites)
    , _activityListStackPosition(StackedWidget::Activity)
{
}

SynthesisPopover::AccountInfoSynthesis::AccountInfoSynthesis(OCC::AccountState *accountState)
    : AccountInfoSynthesis()
{
    initQuotaInfo(accountState);
}

SynthesisPopover::AccountInfoSynthesis::~AccountInfoSynthesis()
{
    delete _synchronizedListWidget;
}

}
