#include "synthesispopover.h"
#include "buttonsbarwidget.h"
#include "bottomwidget.h"
#include "custompushbutton.h"
#include "customtoolbutton.h"
#include "synchronizeditem.h"
#include "synchronizeditemwidget.h"

#include <QBoxLayout>
#include <QDateTime>
#include <QGuiApplication>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPainter>
#include <QPainterPath>
#include <QPolygon>
#include <QScreen>

namespace KDC {

static const int triangleHeight = 10;
static const int triangleWidth  = 20;
static const int trianglePosition = 100; // Position from side
static const int cornerRadius = 5;
static const int toolBarHMargin = 10;
static const int toolBarVMargin = 10;
static const int driveBarHMargin = 10;
static const int driveBarVMargin = 10;
static const int driveBarSpacing = 15;
static const int logoIconSize = 30;

SynthesisPopover::SynthesisPopover(QWidget *parent)
    : QDialog(parent)
    , _sysTrayIconPosition(QPoint(0, 0))
    , _backgroundMainColor(QColor())
    , _driveSelectionWidget(nullptr)
    , _progressBarWidget(nullptr)
    , _statusBarWidget(nullptr)
    , _stackedWidget(nullptr)
    , _synchronizedListWidget(nullptr)
{
    setModal(true);
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);

    init();
    load();
}

void SynthesisPopover::setSysTrayIconPosition(const QPoint &sysTrayIconPosition)
{
    _sysTrayIconPosition = sysTrayIconPosition;
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

    QRect widgetRect = rect();
    QSize screenSize = QGuiApplication::screenAt(_sysTrayIconPosition)->size();
    bool trianglePositionLeft = _sysTrayIconPosition.x() + widgetRect.width() - trianglePosition < screenSize.width();
    bool trianglePositionTop = _sysTrayIconPosition.y() < screenSize.height() - _sysTrayIconPosition.y();

    // Position of the dialog (left/top corner)
    QPoint popoverPosition(
        trianglePositionLeft
        ? _sysTrayIconPosition.x() - trianglePosition
        : _sysTrayIconPosition.x() - widgetRect.width() + trianglePosition,
        trianglePositionTop
        ? _sysTrayIconPosition.y()
        : _sysTrayIconPosition.y() - widgetRect.height());
    move(popoverPosition);

    // Triangle
    QPointF triangleLeftPoint(
        trianglePositionLeft
        ? trianglePosition - triangleWidth / 2.0
        : widgetRect.width() - trianglePosition - triangleWidth / 2.0,
        triangleHeight);
    QPointF triangleRightPoint(
        trianglePositionLeft
        ? trianglePosition + triangleWidth / 2.0
        : widgetRect.width() - trianglePosition + triangleWidth / 2.0,
        triangleHeight);
    QPointF triangleTopPoint(
        trianglePositionLeft
        ? trianglePosition
        : widgetRect.width() - trianglePosition,
        0);
    QPolygonF triangle;
    triangle << triangleLeftPoint << triangleTopPoint << triangleRightPoint;

    // Round rectangle
    QPainterPath painterPath;
    painterPath.addRoundedRect(
                QRectF(0, triangleHeight, widgetRect.width(), widgetRect.height() - triangleHeight),
                cornerRadius, cornerRadius);
    painterPath.addPolygon(triangle);

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
    mainVBox->setContentsMargins(0, 0, 0, 0);
    mainVBox->setSpacing(0);
    mainVBox->addSpacing(triangleHeight);

    // Tool bar
    QHBoxLayout *hboxToolBar = new QHBoxLayout(this);
    hboxToolBar->setContentsMargins(toolBarHMargin, toolBarVMargin, toolBarHMargin, toolBarVMargin);
    hboxToolBar->setSpacing(0);
    mainVBox->addLayout(hboxToolBar);

    QLabel *iconLabel = new QLabel(this);
    iconLabel->setPixmap(QIcon(":/client/resources/logos/kdrive-without-text.svg").pixmap(logoIconSize, logoIconSize));
    hboxToolBar->addWidget(iconLabel);

    QWidget *spacerWidget = new QWidget(this);
    spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    hboxToolBar->addWidget(spacerWidget);

    CustomToolButton *folderButton = new CustomToolButton(this);
    folderButton->setIconPath(":/client/resources/icons/actions/folder.svg");
    hboxToolBar->addWidget(folderButton);

    CustomToolButton *webviewButton = new CustomToolButton(this);
    webviewButton->setIconPath(":/client/resources/icons/actions/webview.svg");
    hboxToolBar->addWidget(webviewButton);

    CustomToolButton *menuButton = new CustomToolButton(this);
    menuButton->setIconPath(":/client/resources/icons/actions/menu.svg");
    hboxToolBar->addWidget(menuButton);

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

    connect(folderButton, &CustomToolButton::clicked, this, &SynthesisPopover::onFolderButtonClicked);
    connect(webviewButton, &CustomToolButton::clicked, this, &SynthesisPopover::onWebviewButtonClicked);
    connect(menuButton, &CustomToolButton::clicked, this, &SynthesisPopover::onMenuButtonClicked);
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
