#include "synthesispopover.h"
#include "halfroundrectwidget.h"
#include "rectwidget.h"
#include "bottomwidget.h"
#include "custompushbutton.h"
#include "customtoolbutton.h"
#include "synchronizeditemdelegate.h"
#include "synchronizeditem.h"
#include "guiutility.h"
#include "theme.h"
#include "syncresult.h"

#include <iostream>
#include <cstdlib>

#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QBrush>
#include <QComboBox>
#include <QDateTime>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QLabel>
#include <QListView>
#include <QPainter>
#include <QPainterPath>
#include <QPolygon>
#include <QProgressBar>
#include <QScreen>
#include <QStackedWidget>
#include <QStandardItem>
#include <QToolBar>

namespace KDC {

static const int triangleHeight = 10;
static const int triangleWidth  = 20;
static const int trianglePosition = 100; // Position from side
static const int cornerRadius = 5;
static const int vMargin = 10;
static const int largeVMargin = 15;
static const int hMargin = 10;
static const int largeHMargin = 20;
static const int hSpacing = 15;
static const int logoSize = 32;
static const int driveIconSize = 20;
static const int statusIconSize = 24;

SynthesisPopover::SynthesisPopover(QWidget *parent)
    : QDialog(parent)
    , _sysTrayIconPosition(QPoint(0, 0))
    , _backgroundMainColor(Qt::white)
{
    setModal(true);
    setWindowFlags(Qt::FramelessWindowHint);

    init();
}

void SynthesisPopover::setSysTrayIconPosition(const QPoint &sysTrayIconPosition)
{
    _sysTrayIconPosition = sysTrayIconPosition;
}

void SynthesisPopover::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    std::cout << "SynthesisPopover::paintEvent" << std::endl;

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

void SynthesisPopover::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);

    //QEvent e(QEvent::WindowActivate);
    //QApplication::postEvent(this, &e);
}

bool SynthesisPopover::event(QEvent *event)
{
    std::cout << "Event: " << event->type() << std::endl;
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
    hboxToolBar->setContentsMargins(vMargin, hMargin, vMargin, 0);
    hboxToolBar->setSpacing(0);
    mainVBox->addLayout(hboxToolBar);

    QLabel *iconLabel = new QLabel(this);
    iconLabel->setPixmap(QIcon(":/client/resources/logos/kdrive-without-text").pixmap(QSize(logoSize, logoSize)));
    hboxToolBar->addWidget(iconLabel);

    QWidget *spacerWidget = new QWidget(this);
    spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacerWidget->setVisible(true);
    hboxToolBar->addWidget(spacerWidget);

    CustomToolButton *folderButton = new CustomToolButton(this);
    folderButton->setIconPath(":/client/resources/icons/actions/folder");
    hboxToolBar->addWidget(folderButton);

    CustomToolButton *webviewButton = new CustomToolButton(this);
    webviewButton->setIconPath(":/client/resources/icons/actions/webview");
    hboxToolBar->addWidget(webviewButton);

    CustomToolButton *menuButton = new CustomToolButton(this);
    menuButton->setIconPath(":/client/resources/icons/actions/menu");
    hboxToolBar->addWidget(menuButton);

    // Drive selection
    QHBoxLayout *hboxDrive = new QHBoxLayout(this);
    hboxDrive->setContentsMargins(largeVMargin, hMargin, largeVMargin, 0);

    QLabel *driveIconLabel = new QLabel(this);
    driveIconLabel->setPixmap(QIcon(":/client/resources/icons/actions/drive").pixmap(QSize(driveIconSize, driveIconSize)));
    hboxDrive->addWidget(driveIconLabel);

    QComboBox *driveComboBox = new QComboBox(this);
    driveComboBox->addItem("FSociety");
    hboxDrive->addWidget(driveComboBox);
    hboxDrive->addStretch();
    mainVBox->addLayout(hboxDrive);

    // Progress bar
    QHBoxLayout *hboxProgressBar = new QHBoxLayout(this);
    hboxProgressBar->setContentsMargins(largeVMargin, hMargin, largeVMargin, 0);
    hboxProgressBar->setSpacing(hSpacing);

    QProgressBar *progressBar = new QProgressBar(this);
    progressBar->setMinimum(0);
    progressBar->setMaximum(100);
    progressBar->setValue(30);
    progressBar->setFormat(QString());
    hboxProgressBar->addWidget(progressBar);

    QLabel *progressLabel = new QLabel("60 Go / 1 To", this);
    hboxProgressBar->addWidget(progressLabel);
    mainVBox->addLayout(hboxProgressBar);

    // Status bar
    HalfRoundRectWidget *statusBarWidget = new HalfRoundRectWidget(this);
    statusBarWidget->getLayout()->setContentsMargins(vMargin, largeHMargin, vMargin, largeHMargin);

    QLabel *statusIconLabel = new QLabel(this);
    statusIconLabel->setPixmap(OCC::Theme::instance()->syncStateIcon(OCC::SyncResult::SyncRunning).pixmap(QSize(statusIconSize, statusIconSize)));
    statusBarWidget->getLayout()->addWidget(statusIconLabel);

    QLabel *statusLabel = new QLabel("Synchronisation en cours (15 sur 200)\n25 minutes restantes...", this);
    statusBarWidget->getLayout()->addWidget(statusLabel);
    statusBarWidget->getLayout()->addStretch();

    CustomToolButton *pauseButton = new CustomToolButton(this);
    pauseButton->setIconPath(":/client/resources/icons/actions/pause");
    statusBarWidget->getLayout()->addWidget(pauseButton);
    mainVBox->addWidget(statusBarWidget);

    // Buttons bar
    RectWidget *buttonsBarWidget = new RectWidget(this);
    buttonsBarWidget->getLayout()->setContentsMargins(largeVMargin, largeHMargin, largeVMargin, largeHMargin);

    CustomPushButton *synchronizedButton = new CustomPushButton(tr("Synchronisés"), buttonsBarWidget);
    synchronizedButton->setIconPath(":/client/resources/icons/actions/sync");
    synchronizedButton->setChecked(true);
    buttonsBarWidget->getLayout()->addWidget(synchronizedButton);

    CustomPushButton *favoritesButton = new CustomPushButton(tr("Favoris"), buttonsBarWidget);
    favoritesButton->setIconPath(":/client/resources/icons/actions/favorite");
    favoritesButton->setChecked(false);
    buttonsBarWidget->getLayout()->addWidget(favoritesButton);

    CustomPushButton *activityButton = new CustomPushButton(tr("Activité"), buttonsBarWidget);
    activityButton->setIconPath(":/client/resources/icons/actions/notifications");
    activityButton->setChecked(false);
    buttonsBarWidget->getLayout()->addWidget(activityButton);

    mainVBox->addWidget(buttonsBarWidget);

    // Stacked widget
    QStackedWidget *stackedWidget = new QStackedWidget(this);

    QStandardItemModel *synchronizedModel = new QStandardItemModel(stackedWidget);
    populateSynchronizedList(synchronizedModel);

    QListView *synchronizedListView = new QListView(stackedWidget);
    synchronizedListView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    synchronizedListView->setSpacing(0);
    synchronizedListView->setModel(synchronizedModel);
    synchronizedListView->setItemDelegate(new SynchronizedItemDelegate(stackedWidget));
    synchronizedListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    synchronizedListView->setCurrentIndex(synchronizedModel->index(0, 0));

    stackedWidget->addWidget(synchronizedListView);

    mainVBox->addWidget(stackedWidget);
    mainVBox->setStretchFactor(stackedWidget, 1);

    // Bottom
    BottomWidget *bottomWidget = new BottomWidget(this);
    mainVBox->addWidget(bottomWidget);

    setLayout(mainVBox);
}

void SynthesisPopover::populateSynchronizedList(QStandardItemModel *model)
{
    for (int i = 0; i < 10; i++) {
        QStandardItem *item1 = new QStandardItem("Picture.jpg");
        SynchronizedItem itemData1 = SynchronizedItem(item1->text(), QDateTime::currentDateTime());
        item1->setData(QVariant::fromValue(itemData1), Qt::DisplayRole);
        model->insertRow(0, item1);
    }

    QStandardItem *item2 = new QStandardItem("Music.mp3");
    SynchronizedItem itemData2 = SynchronizedItem(item2->text(), QDateTime::currentDateTime());
    item2->setData(QVariant::fromValue(itemData2), Qt::DisplayRole);
    model->insertRow(0, item2);

    QStandardItem *item3 = new QStandardItem("Rapport des ventes - mai 2019.pdf");
    SynchronizedItem itemData3 = SynchronizedItem(item3->text(), QDateTime::currentDateTime());
    item3->setData(QVariant::fromValue(itemData3), Qt::DisplayRole);
    model->insertRow(0, item3);
}

}
