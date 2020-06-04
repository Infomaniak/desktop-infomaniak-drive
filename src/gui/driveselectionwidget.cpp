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

#include "driveselectionwidget.h"
#include "menuitemwidget.h"
#include "menuwidget.h"
#include "guiutility.h"

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
static const char driveIdProperty[] = "driveId";
static const int driveNameMaxSize = 30;

DriveSelectionWidget::DriveSelectionWidget(QWidget *parent)
    : QPushButton(parent)
    , _driveIconSize(QSize())
    , _downIconSize(QSize())
    , _downIconColor(QColor())
    , _menuRightIconSize(QSize())
    , _currentDriveId(QString())
    , _driveIconLabel(nullptr)
    , _driveTextLabel(nullptr)
    , _downIconLabel(nullptr)
{
    setContentsMargins(hMargin, vMargin, hMargin, vMargin);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->setContentsMargins(boxHMargin, boxVMargin, boxHMargin, boxVMargin);
    hbox->setSpacing(boxSpacing);
    setLayout(hbox);

    _driveIconLabel = new QLabel(this);
    hbox->addWidget(_driveIconLabel);

    _driveTextLabel = new QLabel(this);
    _driveTextLabel->setObjectName("driveTextLabel");
    hbox->addWidget(_driveTextLabel);
    hbox->addStretch();

    _downIconLabel = new QLabel(this);
    hbox->addWidget(_downIconLabel);

    connect(this, &DriveSelectionWidget::driveIconSizeChanged, this, &DriveSelectionWidget::onDriveIconSizeChanged);
    connect(this, &DriveSelectionWidget::downIconSizeChanged, this, &DriveSelectionWidget::onDownIconSizeChanged);
    connect(this, &DriveSelectionWidget::downIconColorChanged, this, &DriveSelectionWidget::onDownIconColorChanged);
    connect(this, &DriveSelectionWidget::clicked, this, &DriveSelectionWidget::onClick);
    connect(this, &DriveSelectionWidget::addIconColorChanged, this, &DriveSelectionWidget::onAddIconColorChanged);
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

void DriveSelectionWidget::clear()
{
    _currentDriveId.clear();
    _driveMap.clear();
    setAddDriveIcon();
    _driveTextLabel->setText(tr("Add a kDrive"));
    _downIconLabel->setVisible(false);
}

void DriveSelectionWidget::addOrUpdateDrive(QString id, const AccountInfo &accountInfo)
{
    _driveMap[id] = accountInfo;
}

void DriveSelectionWidget::removeDrive(QString id)
{
    auto driveInfoIt = _driveMap.find(id);
    if (driveInfoIt != _driveMap.end()) {
        _driveMap.erase(driveInfoIt);

        // Select 1st drive
        driveInfoIt = _driveMap.begin();
        if (driveInfoIt != _driveMap.end()) {
            selectDrive(driveInfoIt->first);
        }
        else {
            clear();
        }
    }
}

void DriveSelectionWidget::selectDrive(QString id)
{
    if (_driveMap.find(id) != _driveMap.end()) {
        QString driveName = _driveMap[id]._name;
        if (driveName.size() > driveNameMaxSize) {
            driveName = driveName.left(driveNameMaxSize)  + "...";
        }
        _driveTextLabel->setText(driveName);
        _downIconLabel->setVisible(true);
        setDriveIcon(_driveMap[id]._color);
        _currentDriveId = id;
        emit driveSelected(id);
    }
}

void DriveSelectionWidget::onDriveIconSizeChanged()
{
    if (_driveMap.find(_currentDriveId) != _driveMap.end()) {
        setDriveIcon(_driveMap[_currentDriveId]._color);
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

void DriveSelectionWidget::onAddIconColorChanged()
{
    setAddDriveIcon();
}

void DriveSelectionWidget::onClick(bool checked)
{
    Q_UNUSED(checked)

    // Remove hover
    QApplication::sendEvent(this, new QEvent(QEvent::Leave));
    QApplication::sendEvent(this, new QEvent(QEvent::HoverLeave));

    if (_driveMap.size() > 0) {
        MenuWidget *menu = new MenuWidget(MenuWidget::List, this);

        for (auto const &driveMapElt : _driveMap) {
            QWidgetAction *selectDriveAction = new QWidgetAction(this);
            selectDriveAction->setProperty(driveIdProperty, driveMapElt.first);
            MenuItemWidget *driveMenuItemWidget = new MenuItemWidget(driveMapElt.second._name);
            driveMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/drive.svg", driveMapElt.second._color);
            driveMenuItemWidget->setRightIcon(
                        QIcon(OCC::Utility::getAccountStatusIconPath(driveMapElt.second._paused, driveMapElt.second._status)),
                        _menuRightIconSize);
            selectDriveAction->setDefaultWidget(driveMenuItemWidget);
            connect(selectDriveAction, &QWidgetAction::triggered, this, &DriveSelectionWidget::onSelectDriveActionTriggered);
            menu->addAction(selectDriveAction);
        }

        QWidgetAction *addDriveAction = new QWidgetAction(this);
        MenuItemWidget *addDriveMenuItemWidget = new MenuItemWidget(tr("Add a kDrive"));
        addDriveMenuItemWidget->setLeftIcon(":/client/resources/icons/actions/add.svg");
        addDriveAction->setDefaultWidget(addDriveMenuItemWidget);
        connect(addDriveAction, &QWidgetAction::triggered, this, &DriveSelectionWidget::onAddDriveActionTriggered);
        menu->addAction(addDriveAction);

        menu->exec(QWidget::mapToGlobal(rect().bottomLeft()));
    }
    else {
        onAddDriveActionTriggered();
    }
}

void DriveSelectionWidget::onSelectDriveActionTriggered(bool checked)
{
    Q_UNUSED(checked)

    QString driveId = qvariant_cast<QString>(sender()->property(driveIdProperty));
    if (driveId != _currentDriveId) {
        selectDrive(driveId);
    }
}

void DriveSelectionWidget::onAddDriveActionTriggered(bool checked)
{
    Q_UNUSED(checked)

    emit addDrive();
}

void DriveSelectionWidget::setDriveIcon(const QColor &color)
{
    if (_driveIconLabel) {
        _driveIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/drive.svg", color).
                                   pixmap(_driveIconSize));
    }
}

void DriveSelectionWidget::setAddDriveIcon()
{
    if (_driveIconLabel && _driveIconSize != QSize() && _addIconColor != QColor()) {
        _driveIconLabel->setPixmap(OCC::Utility::getIconWithColor(":/client/resources/icons/actions/add.svg", _addIconColor).
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
