#include "synthesispopover.h"
#include "menuwidget.h"
#include "buttonsbarwidget.h"
#include "bottomwidget.h"
#include "custompushbutton.h"
#include "synchronizeditem.h"
#include "synchronizeditemwidget.h"
#include "guiutility.h"

#include <QAction>
#include <QBoxLayout>
#include <QDateTime>
#include <QGraphicsDropShadowEffect>
#include <QGuiApplication>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>

namespace KDC {

static const int triangleHeight = 10;
static const int triangleWidth  = 20;
static const int trianglePosition = 100; // Position from side
static const int cornerRadius = 5;
static const int shadowBlurRadius = 40;
static const int toolBarHMargin = 10;
static const int toolBarVMargin = 10;
static const int toolBarSpacing = 10;
static const int driveBarHMargin = 10;
static const int driveBarVMargin = 10;
static const int driveBarSpacing = 15;
static const int logoIconSize = 30;

SynthesisPopover::SynthesisPopover(QWidget *parent)
    : QDialog(parent)
    , _sysTrayIconRect(QRect())
    , _backgroundMainColor(QColor())
    , _folderButton(nullptr)
    , _webviewButton(nullptr)
    , _menuButton(nullptr)
    , _driveSelectionWidget(nullptr)
    , _progressBarWidget(nullptr)
    , _statusBarWidget(nullptr)
    , _stackedWidget(nullptr)
    , _synchronizedListWidget(nullptr)
{
    setModal(true);
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::X11BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground);

    setVisible(false);
    init();
    load();
    setVisible(true);
}

void SynthesisPopover::setSysTrayIconRect(const QRect &sysTrayIconRect)
{
    _sysTrayIconRect = sysTrayIconRect;
}

void SynthesisPopover::setTransferTotalSize(long size)
{
    if (_progressBarWidget) {
        _progressBarWidget->setTransferTotalSize(size);
    }
}

void SynthesisPopover::setTransferSize(long size)
{
    if (_progressBarWidget) {
        _progressBarWidget->setTransferSize(size);
    }
}

void SynthesisPopover::setStatus(OCC::SyncResult::Status status, int fileNum, int fileCount, const QTime &time)
{
    if (_statusBarWidget) {
        _statusBarWidget->setStatus(status, fileNum, fileCount, time);
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
    if (_sysTrayIconRect == QRect()) {
        // Unknown Systray icon position

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
        done(0);
        event->ignore();
        return true;
    }
    return QWidget::event(event);
}

void SynthesisPopover::init()
{
    QVBoxLayout *mainVBox = new QVBoxLayout(this);
    mainVBox->setContentsMargins(triangleHeight, triangleHeight, triangleHeight, triangleHeight);
    mainVBox->setSpacing(0);

    // Tool bar
    QHBoxLayout *hboxToolBar = new QHBoxLayout(this);
    hboxToolBar->setContentsMargins(toolBarHMargin, toolBarVMargin, toolBarHMargin, toolBarVMargin);
    hboxToolBar->setSpacing(toolBarSpacing);
    mainVBox->addLayout(hboxToolBar);

    QLabel *iconLabel = new QLabel(this);
    iconLabel->setPixmap(QIcon(":/client/resources/logos/kdrive-without-text.svg").pixmap(logoIconSize, logoIconSize));
    hboxToolBar->addWidget(iconLabel);

    QWidget *spacerWidget = new QWidget(this);
    spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    hboxToolBar->addWidget(spacerWidget);

    _folderButton = new CustomToolButton(this);
    _folderButton->setIconPath(":/client/resources/icons/actions/folder.svg");
    hboxToolBar->addWidget(_folderButton);

    _webviewButton = new CustomToolButton(this);
    _webviewButton->setIconPath(":/client/resources/icons/actions/webview.svg");
    hboxToolBar->addWidget(_webviewButton);

    _menuButton = new CustomToolButton(this);
    _menuButton->setIconPath(":/client/resources/icons/actions/menu.svg");
    hboxToolBar->addWidget(_menuButton);

    // Drive selection
    QHBoxLayout *hboxDriveBar = new QHBoxLayout(this);
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

    CustomPushButton *synchronizedButton = new CustomPushButton(tr("Synchronisés"), buttonsBarWidget);
    synchronizedButton->setIconPath(":/client/resources/icons/actions/sync.svg");
    buttonsBarWidget->insertButton(stackedWidget::SynchronizedItems, synchronizedButton);

    CustomPushButton *favoritesButton = new CustomPushButton(tr("Favoris"), buttonsBarWidget);
    favoritesButton->setIconPath(":/client/resources/icons/actions/favorite.svg");
    buttonsBarWidget->insertButton(stackedWidget::Favorites, favoritesButton);

    CustomPushButton *activityButton = new CustomPushButton(tr("Activité"), buttonsBarWidget);
    activityButton->setIconPath(":/client/resources/icons/actions/notifications.svg");
    buttonsBarWidget->insertButton(stackedWidget::Activity, activityButton);

    mainVBox->addWidget(buttonsBarWidget);

    // Stacked widget
    _stackedWidget = new QStackedWidget(this);

    _synchronizedListWidget = new QListWidget(this);
    _synchronizedListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    _synchronizedListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    _synchronizedListWidget->setSpacing(0);

    _stackedWidget->insertWidget(stackedWidget::SynchronizedItems, _synchronizedListWidget);

    QWidget *favoritesWidget = new QWidget(this);
    _stackedWidget->insertWidget(stackedWidget::Favorites, favoritesWidget);

    QWidget *activityWidget = new QWidget(this);
    _stackedWidget->insertWidget(stackedWidget::Activity, activityWidget);

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

    connect(_folderButton, &CustomToolButton::clicked, this, &SynthesisPopover::onFolderButtonClicked);
    connect(_webviewButton, &CustomToolButton::clicked, this, &SynthesisPopover::onWebviewButtonClicked);
    connect(_menuButton, &CustomToolButton::clicked, this, &SynthesisPopover::onMenuButtonClicked);
    connect(_driveSelectionWidget, &DriveSelectionWidget::driveSelected, this, &SynthesisPopover::onDriveSelected);
    connect(buttonsBarWidget, &ButtonsBarWidget::buttonToggled, this, &SynthesisPopover::onButtonBarToggled);
    connect(_synchronizedListWidget, &QListWidget::currentItemChanged, this, &SynthesisPopover::onCurrentItemChanged);
}

void SynthesisPopover::load()
{
    loadDriveList();
    setTransferTotalSize(1000000);
    setTransferSize(60000);
    setStatus(OCC::SyncResult::SyncRunning, 15, 200, QTime(0, 25));
    loadSynchronizedList();
}

void SynthesisPopover::loadDriveList()
{
    _driveSelectionWidget->addDrive(111111, "FSociety", QColor("#666666"));
    _driveSelectionWidget->addDrive(222222, "Perso", QColor("#FF0000"));
    _driveSelectionWidget->selectDrive(111111);
}

void SynthesisPopover::loadSynchronizedList()
{
    QListWidgetItem *item;
    SynchronizedItemWidget *widget;

    item = new QListWidgetItem(_synchronizedListWidget);
    SynchronizedItem itemData3 = SynchronizedItem("Rapport des ventes - mai 2019.pdf", QDateTime::currentDateTime());
    _synchronizedListWidget->insertItem(0, item);
    widget = new SynchronizedItemWidget(itemData3, _synchronizedListWidget);
    _synchronizedListWidget->setItemWidget(item, widget);

    item = new QListWidgetItem(_synchronizedListWidget);
    SynchronizedItem itemData2 = SynchronizedItem("Music.mp3", QDateTime::currentDateTime());
    _synchronizedListWidget->insertItem(0, item);
    widget = new SynchronizedItemWidget(itemData2, _synchronizedListWidget);
    _synchronizedListWidget->setItemWidget(item, widget);

    for (int i = 0; i < 10; i++) {
        item = new QListWidgetItem(_synchronizedListWidget);
        _synchronizedListWidget->insertItem(0, item);
        SynchronizedItem itemData1 = SynchronizedItem("Picture.jpg", QDateTime::currentDateTime());
        widget = new SynchronizedItemWidget(itemData1, _synchronizedListWidget);
        _synchronizedListWidget->setItemWidget(item, widget);
    }

    _synchronizedListWidget->setCurrentRow(0);
}

void SynthesisPopover::onFolderButtonClicked()
{

}

void SynthesisPopover::onWebviewButtonClicked()
{

}

void SynthesisPopover::onMenuButtonClicked()
{
    if (_menuButton) {
        MenuWidget *menu = new MenuWidget(this);

        QAction *parametersAction = new QAction(tr("Parameters"), this);
        parametersAction->setProperty(MenuWidget::iconPathProperty.c_str(), ":/client/resources/icons/actions/parameters.svg");
        connect(parametersAction, &QAction::triggered, this, &SynthesisPopover::onParametersActionTriggered);
        menu->addAction(parametersAction);

        QAction *notificationsAction = new QAction(tr("Disable Notifications"), this);
        notificationsAction->setProperty(MenuWidget::iconPathProperty.c_str(), ":/client/resources/icons/actions/notification-off.svg");
        connect(notificationsAction, &QAction::triggered, this, &SynthesisPopover::onNotificationsActionTriggered);
        menu->addAction(notificationsAction);

        QAction *helpAction = new QAction(tr("Need help"), this);
        helpAction->setProperty(MenuWidget::iconPathProperty.c_str(), ":/client/resources/icons/actions/help.svg");
        connect(helpAction, &QAction::triggered, this, &SynthesisPopover::onHelpActionTriggered);
        menu->addAction(helpAction);
        menu->addSeparator();

        QAction *exitAction = new QAction(tr("Quit application"), this);
        exitAction->setProperty(MenuWidget::iconPathProperty.c_str(), ":/client/resources/icons/actions/error-sync.svg");
        connect(exitAction, &QAction::triggered, this, &SynthesisPopover::onExitActionTriggered);
        menu->addAction(exitAction);

        menu->exec(geometry().topLeft() + _menuButton->geometry().bottomLeft() + QPoint(-20, 10));
    }
}

void SynthesisPopover::onParametersActionTriggered(bool checked)
{
    Q_UNUSED(checked)
}

void SynthesisPopover::onNotificationsActionTriggered(bool checked)
{
    Q_UNUSED(checked)
}

void SynthesisPopover::onHelpActionTriggered(bool checked)
{
    Q_UNUSED(checked)
}

void SynthesisPopover::onExitActionTriggered(bool checked)
{
    Q_UNUSED(checked)
}

void SynthesisPopover::onDriveSelected(int id)
{
    Q_UNUSED(id)
}

void SynthesisPopover::onButtonBarToggled(int position)
{
    _stackedWidget->setCurrentIndex(position);
}

void SynthesisPopover::onCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (previous) {
        SynchronizedItemWidget *previousWidget =
                dynamic_cast<SynchronizedItemWidget *>(_synchronizedListWidget->itemWidget(previous));
        previousWidget->setSelected(false);
    }

    if (current) {
        SynchronizedItemWidget *currentWidget =
                dynamic_cast<SynchronizedItemWidget *>(_synchronizedListWidget->itemWidget(current));
        currentWidget->setSelected(true);
    }
}

}
