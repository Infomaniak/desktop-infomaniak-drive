#include "driveselectionwidget.h"
#include "guiutility.h"

#include <QIcon>
#include <QHBoxLayout>
#include <QLabel>
#include <QSizePolicy>

namespace KDC {

static const int hMargin= 0;
static const int vMargin = 0;
static const int boxHMargin= 10;
static const int boxVMargin = 5;
static const int boxSpacing = 10;

DriveSelectionWidget::DriveSelectionWidget(QWidget *parent)
    : QPushButton(parent)
    , _driveIconSize(QSize())
    , _downIconSize(QSize())
    , _downIconColor(QColor())
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

void DriveSelectionWidget::addDrive(int id, const QString &name, const QColor &color)
{
    _driveMap[id] = std::make_pair(name, color);
}

void DriveSelectionWidget::selectDrive(int id)
{
    if (_driveMap.find(id) != _driveMap.end()) {
        _currentDriveId = id;
        _driveTextLabel->setText(_driveMap[id].first);
        setDriveIcon(_driveMap[id].second);
        emit driveSelected(id);
    }
}

void DriveSelectionWidget::onDriveIconSizeChanged()
{
    if (_driveMap.find(_currentDriveId) != _driveMap.end()) {
        setDriveIcon(_driveMap[_currentDriveId].second);
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
