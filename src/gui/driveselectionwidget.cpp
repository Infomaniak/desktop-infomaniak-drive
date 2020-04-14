#include "driveselectionwidget.h"
#include "menuitemwidget.h"
#include "menuwidget.h"
#include "guiutility.h"

#include <QIcon>
#include <QHBoxLayout>
#include <QLabel>
#include <QSizePolicy>
#include <QWidgetAction>

namespace KDC {

static const int hMargin = 0;
static const int vMargin = 0;
static const int boxHMargin= 10;
static const int boxVMargin = 5;
static const int boxSpacing = 10;

DriveSelectionWidget::DriveSelectionWidget(QWidget *parent)
    : QPushButton(parent)
    , _driveIconSize(QSize())
    , _downIconSize(QSize())
    , _downIconColor(QColor())
    , _menuRightIconSize(QSize())
    , _currentDriveId(0)
    , _driveIconLabel(nullptr)
    , _driveTextLabel(nullptr)
    , _downIconLabel(nullptr)
{
    setContentsMargins(hMargin, vMargin, hMargin, vMargin);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QHBoxLayout *hbox = new QHBoxLayout(this);
    hbox->setContentsMargins(boxHMargin, boxVMargin, boxHMargin, boxVMargin);
    hbox->setSpacing(boxSpacing);
    setLayout(hbox);

    _driveIconLabel = new QLabel(this);
    hbox->addWidget(_driveIconLabel);

    _driveTextLabel = new QLabel(this);
    _driveTextLabel->setObjectName("driveTextLabel");
    hbox->addWidget(_driveTextLabel);

    _downIconLabel = new QLabel(this);
    hbox->addWidget(_downIconLabel);

    connect(this, &DriveSelectionWidget::driveIconSizeChanged, this, &DriveSelectionWidget::onDriveIconSizeChanged);
    connect(this, &DriveSelectionWidget::downIconSizeChanged, this, &DriveSelectionWidget::onDownIconSizeChanged);
    connect(this, &DriveSelectionWidget::downIconColorChanged, this, &DriveSelectionWidget::onDownIconColorChanged);
    connect(this, &DriveSelectionWidget::clicked, this, &DriveSelectionWidget::onClick);
}

QSize DriveSelectionWidget::sizeHint() const
{
    return QSize(_driveIconLabel->sizeHint().width()
                 + _driveTextLabel->sizeHint().width()
                 + _downIconLabel->sizeHint().width()
                 + 2 * boxSpacing
                 + 2 * boxHMargin,
                 QPushButton::sizeHint().height());
}

void DriveSelectionWidget::addDrive(int id, const QString &name, const QColor &color, OCC::SyncResult::Status status)
{
    _driveMap[id] = std::make_tuple(name, color, status);
}

void DriveSelectionWidget::selectDrive(int id)
{
    if (_driveMap.find(id) != _driveMap.end()) {
        _currentDriveId = id;
        _driveTextLabel->setText(std::get<0>(_driveMap[id]));
        setDriveIcon(std::get<1>(_driveMap[id]));
        emit driveSelected(id);
    }
}

void DriveSelectionWidget::onDriveIconSizeChanged()
{
    if (_driveMap.find(_currentDriveId) != _driveMap.end()) {
        setDriveIcon(std::get<1>(_driveMap[_currentDriveId]));
    }
}

void DriveSelectionWidget::onDownIconSizeChanged()
{
    setDownIcon();
}

void DriveSelectionWidget::onDownIconColorChanged()
{
    setDownIcon();
}

void DriveSelectionWidget::onClick(bool checked)
{
    Q_UNUSED(checked)

    MenuWidget *menu = new MenuWidget(this);

    for (auto drive : _driveMap) {
        if (drive.first != _currentDriveId) {
            QWidgetAction *selectDriveAction = new QWidgetAction(this);
            MenuItemWidget *driveMenuItemWidget = new MenuItemWidget(std::get<0>(drive.second));
            driveMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/drive.svg", std::get<1>(drive.second));
            driveMenuItemWidget->setRightIcon(OCC::Theme::instance()->syncStateIcon(std::get<2>(drive.second)), _menuRightIconSize);
            selectDriveAction->setDefaultWidget(driveMenuItemWidget);
            connect(selectDriveAction, &QWidgetAction::triggered, this, &DriveSelectionWidget::onSelectDriveActionTriggered);
            menu->addAction(selectDriveAction);
        }
    }

    QWidgetAction *addDriveAction = new QWidgetAction(this);
    MenuItemWidget *addDriveMenuItemWidget = new MenuItemWidget(tr("Add a kDrive"));
    addDriveMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/add.svg");
    addDriveAction->setDefaultWidget(addDriveMenuItemWidget);
    connect(addDriveAction, &QWidgetAction::triggered, this, &DriveSelectionWidget::onAddDriveActionTriggered);
    menu->addAction(addDriveAction);

    menu->exec(QWidget::mapToGlobal(rect().bottomLeft()));
}

void DriveSelectionWidget::onSelectDriveActionTriggered(bool checked)
{
    Q_UNUSED(checked)
}

void DriveSelectionWidget::onAddDriveActionTriggered(bool checked)
{
    Q_UNUSED(checked)
}

void DriveSelectionWidget::setDriveIcon(const QColor &color)
{
    if (_driveIconLabel) {
        _driveIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/drive.svg", color).
                                   pixmap(_driveIconSize));
    }
}

void DriveSelectionWidget::setDownIcon()
{
    if (_downIconLabel && _downIconSize != QSize() && _downIconColor != QColor()) {
        _downIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/chevron-down.svg", _downIconColor).
                                  pixmap(_downIconSize));
    }
}

}
