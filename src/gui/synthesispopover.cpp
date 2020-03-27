#include "synthesispopover.h"
#include "halfroundrectwidget.h"
#include "rectwidget.h"
#include "synthesisitemdelegate.h"
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
#include <QGraphicsColorizeEffect>
#include <QLabel>
#include <QListView>
#include <QPainter>
#include <QPainterPath>
#include <QPolygon>
#include <QProgressBar>
#include <QPushButton>
#include <QScreen>
#include <QStackedWidget>
#include <QStandardItem>
#include <QToolBar>
#include <QToolButton>
#include <QVariant>

namespace KDC {

static const int triangleHeight = 10;
static const int triangleWidth  = 20;
static const int trianglePosition = 100; // Position from side
static const int cornerRadius = 5;
static const int verticalMargin = 10;
static const int largeVerticalMargin = 15;
static const int horizontalMargin = 10;
static const int largeHorizontalMargin = 20;
static const int horizontalSpacing = 15;
static const int logoSize = 32;

SynthesisPopover::SynthesisPopover(QWidget *parent)
    : QDialog(parent)
    , _sysTrayIconPosition(QPoint(0, 0))
    , _backgroundMainColor(Qt::white)
{
    setModal(true);
    setWindowFlags(Qt::FramelessWindowHint);

    QVBoxLayout *mainVBox = new QVBoxLayout(this);
    mainVBox->setContentsMargins(0, 0, 0, 0);
    mainVBox->setSpacing(0);
    mainVBox->addSpacing(triangleHeight);

    // Tool bar
    QHBoxLayout *hboxToolBar = new QHBoxLayout(this);
    hboxToolBar->setContentsMargins(verticalMargin, horizontalMargin, verticalMargin, 0);
    hboxToolBar->setSpacing(0);
    mainVBox->addLayout(hboxToolBar);

    QLabel *iconLabel = new QLabel(this);
    iconLabel->setPixmap(QIcon(":/client/resources/logos/kdrive").pixmap(QSize(logoSize, logoSize)));
    hboxToolBar->addWidget(iconLabel);

    QWidget *spacerWidget = new QWidget(this);
    spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacerWidget->setVisible(true);
    hboxToolBar->addWidget(spacerWidget);

    QToolButton *folderButton = new QToolButton(this);
    folderButton->setIcon(QIcon(":/client/resources/icons/actions/folder"));
    OCC::Utility::applyThemeColor(folderButton);
    hboxToolBar->addWidget(folderButton);

    QToolButton *webviewButton = new QToolButton(this);
    webviewButton->setIcon(QIcon(":/client/resources/icons/actions/webview"));
    OCC::Utility::applyThemeColor(webviewButton);
    hboxToolBar->addWidget(webviewButton);

    QToolButton *settingsButton = new QToolButton(this);
    settingsButton->setIcon(QIcon(":/client/resources/icons/actions/parameters"));
    OCC::Utility::applyThemeColor(settingsButton);
    hboxToolBar->addWidget(settingsButton);

    // Drive selection
    QHBoxLayout *hboxDrive = new QHBoxLayout(this);
    hboxDrive->setContentsMargins(largeVerticalMargin, horizontalMargin, largeVerticalMargin, 0);

    QComboBox *driveComboBox = new QComboBox(this);
    driveComboBox->addItem("FSociety");
    hboxDrive->addWidget(driveComboBox);
    hboxDrive->addStretch();
    mainVBox->addLayout(hboxDrive);

    // Progress bar
    QHBoxLayout *hboxProgressBar = new QHBoxLayout(this);
    hboxProgressBar->setContentsMargins(largeVerticalMargin, horizontalMargin, largeVerticalMargin, 0);
    hboxProgressBar->setSpacing(horizontalSpacing);

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
    statusBarWidget->getLayout()->setContentsMargins(verticalMargin, largeHorizontalMargin, verticalMargin, largeHorizontalMargin);

    QToolButton *statusButton = new QToolButton(statusBarWidget);
    statusButton->setIcon(OCC::Theme::instance()->syncStateIcon(OCC::SyncResult::SyncRunning));
    statusBarWidget->getLayout()->addWidget(statusButton);

    QLabel *statusLabel = new QLabel("Synchronisation en cours (15 sur 200)\n25 minutes restantes...", statusBarWidget);
    statusBarWidget->getLayout()->addWidget(statusLabel);

    QToolButton *pauseButton = new QToolButton(statusBarWidget);
    pauseButton->setIcon(QIcon(":/client/resources/settings"));
    statusBarWidget->getLayout()->addWidget(pauseButton);
    mainVBox->addWidget(statusBarWidget);

    // Buttons bar
    RectWidget *buttonsBarWidget = new RectWidget(this);
    buttonsBarWidget->getLayout()->setContentsMargins(largeVerticalMargin, largeHorizontalMargin, largeVerticalMargin, largeHorizontalMargin);

    QPushButton *synchronizedButton = new QPushButton(QIcon(":/client/resources/network"), "Synchronisés", buttonsBarWidget);
    synchronizedButton->setFlat(true);
    synchronizedButton->setCheckable(true);
    synchronizedButton->setChecked(true);
    buttonsBarWidget->getLayout()->addWidget(synchronizedButton);

    QPushButton *favoritesButton = new QPushButton(QIcon(":/client/resources/network"), "Favoris", buttonsBarWidget);
    favoritesButton->setFlat(true);
    favoritesButton->setCheckable(true);
    favoritesButton->setChecked(false);
    buttonsBarWidget->getLayout()->addWidget(favoritesButton);

    QPushButton *activityButton = new QPushButton(QIcon(":/client/resources/network"), "Activité", buttonsBarWidget);
    activityButton->setFlat(true);
    activityButton->setCheckable(true);
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
    synchronizedListView->setItemDelegate(new SynthesisItemDelegate(stackedWidget));
    synchronizedListView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    stackedWidget->addWidget(synchronizedListView);

    mainVBox->addWidget(stackedWidget);
    mainVBox->setStretchFactor(stackedWidget, 1);
    mainVBox->addSpacing(cornerRadius);

    setLayout(mainVBox);
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
    painter.setPen(Qt::NoPen); // No border
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

void SynthesisPopover::populateSynchronizedList(QStandardItemModel *model)
{
    for (int i = 0; i < 100; i++) {
        QStandardItem *item1 = new QStandardItem("Picture.jpg");
        SynchronizedItem itemData1 = SynchronizedItem(item1->text(), QDate());
        item1->setData(QVariant::fromValue(itemData1), Qt::DisplayRole);
        model->insertRow(0, item1);
    }

    QStandardItem *item2 = new QStandardItem("Music.mp3");
    SynchronizedItem itemData2 = SynchronizedItem(item2->text(), QDate());
    item2->setData(QVariant::fromValue(itemData2), Qt::DisplayRole);
    model->insertRow(0, item2);

    QStandardItem *item3 = new QStandardItem("Rapport des ventes - mai 2019.pdf");
    SynchronizedItem itemData3 = SynchronizedItem(item3->text(), QDate());
    item3->setData(QVariant::fromValue(itemData3), Qt::DisplayRole);
    model->insertRow(0, item3);
}

}
