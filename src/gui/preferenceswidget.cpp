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

#include "preferenceswidget.h"
#include "preferencesblocwidget.h"
#include "customswitch.h"
#include "fileexclusiondialog.h"
#include "proxyserverdialog.h"
#include "configfile.h"
#include "common/utility.h"
#include "theme.h"

#include <QBoxLayout>
#include <QIntValidator>
#include <QLabel>

namespace KDC {

static const int boxHMargin= 20;
static const int boxVMargin = 20;
static const int boxSpacing = 12;
static const int textHSpacing = 10;
static const int amountLineEditWidth = 85;

PreferencesWidget::PreferencesWidget(QWidget *parent)
    : QWidget(parent)
    , _folderConfirmationAmountLineEdit(nullptr)
{
    setContentsMargins(0, 0, 0, 0);

    OCC::ConfigFile cfg;

    /*
     *  vBox
     *      generalLabel
     *      generalBloc
     *          folderConfirmationBox
     *              folderConfirmation1HBox
     *                  folderConfirmationLabel
     *                  folderConfirmationSwitch
     *              folderConfirmation2HBox
     *                  _folderConfirmationAmountLineEdit
     *                  folderConfirmationAmountLabel
     *          darkThemeBox
     *              darkThemeLabel
     *              darkThemeSwitch
     *          monochromeIconsBox
     *              monochromeLabel
     *              monochromeSwitch
     *          launchAtStartupBox
     *              launchAtStartupLabel
     *              launchAtStartupSwitch
     *      advancedLabel
     *      advancedBloc
     *          filesToExcludeWidget
     *              filesToExcludeVBox
     *                  filesToExcludeLabel
     *          proxyServerWidget
     *              proxyServerVBox
     *                  proxyServerLabel
     *          bandwidthWidget
     *              bandwidthVBox
     *                  bandwidthLabel
     */

    QVBoxLayout *vBox = new QVBoxLayout();
    vBox->setContentsMargins(boxHMargin, boxVMargin, boxHMargin, boxVMargin);
    vBox->setSpacing(boxSpacing);
    setLayout(vBox);

    //
    // General bloc
    //
    QLabel *generalLabel = new QLabel(tr("General"), this);
    generalLabel->setObjectName("blocLabel");
    vBox->addWidget(generalLabel);

    PreferencesBlocWidget *generalBloc = new PreferencesBlocWidget(this);
    vBox->addWidget(generalBloc);

    // Folder synchronization confirmation
    QBoxLayout *folderConfirmationBox = generalBloc->addLayout(QBoxLayout::Direction::TopToBottom);

    QHBoxLayout *folderConfirmation1HBox = new QHBoxLayout();
    folderConfirmation1HBox->setContentsMargins(0, 0, 0, 0);
    folderConfirmation1HBox->setSpacing(0);
    folderConfirmationBox->addLayout(folderConfirmation1HBox);

    QLabel *folderConfirmationLabel = new QLabel(tr("Ask for confirmation before synchronizing files greater than"), this);
    folderConfirmation1HBox->addWidget(folderConfirmationLabel);
    folderConfirmation1HBox->addStretch();

    CustomSwitch *folderConfirmationSwitch = new CustomSwitch(this);
    folderConfirmationSwitch->setLayoutDirection(Qt::RightToLeft);
    folderConfirmationSwitch->setAttribute(Qt::WA_MacShowFocusRect, false);
    auto folderLimit = cfg.newBigFolderSizeLimit();
    folderConfirmationSwitch->setCheckState(folderLimit.first ? Qt::Checked : Qt::Unchecked);
    folderConfirmation1HBox->addWidget(folderConfirmationSwitch);

    QHBoxLayout *folderConfirmation2HBox = new QHBoxLayout();
    folderConfirmation2HBox->setContentsMargins(0, 0, 0, 0);
    folderConfirmation2HBox->setSpacing(textHSpacing);
    folderConfirmationBox->addLayout(folderConfirmation2HBox);

    _folderConfirmationAmountLineEdit = new QLineEdit(this);
    _folderConfirmationAmountLineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    _folderConfirmationAmountLineEdit->setEnabled(folderLimit.first);
    _folderConfirmationAmountLineEdit->setText(QString::number(folderLimit.second));
    _folderConfirmationAmountLineEdit->setValidator(new QIntValidator(0, 999999, this));
    _folderConfirmationAmountLineEdit->setMinimumWidth(amountLineEditWidth);
    _folderConfirmationAmountLineEdit->setMaximumWidth(amountLineEditWidth);
    folderConfirmation2HBox->addWidget(_folderConfirmationAmountLineEdit);

    QLabel *folderConfirmationAmountLabel = new QLabel("Mo", this);
    folderConfirmationAmountLabel->setObjectName("folderConfirmationAmountLabel");
    folderConfirmation2HBox->addWidget(folderConfirmationAmountLabel);
    folderConfirmation2HBox->addStretch();
    generalBloc->addSeparator();

    // Dark theme activation
    QBoxLayout *darkThemeBox = generalBloc->addLayout(QBoxLayout::Direction::LeftToRight);

    QLabel *darkThemeLabel = new QLabel(tr("Activate dark theme"), this);
    darkThemeBox->addWidget(darkThemeLabel);
    darkThemeBox->addStretch();

    CustomSwitch *darkThemeSwitch = new CustomSwitch(this);
    darkThemeSwitch->setLayoutDirection(Qt::RightToLeft);
    darkThemeSwitch->setAttribute(Qt::WA_MacShowFocusRect, false);
    if (OCC::Utility::isMac()) {
        bool darkSystray = OCC::Utility::hasDarkSystray();
        darkThemeSwitch->setCheckState(darkSystray ? Qt::Checked : Qt::Unchecked);
        darkThemeSwitch->setDisabled(true);
    }
    else {
        darkThemeSwitch->setCheckState(cfg.darkTheme() ? Qt::Checked : Qt::Unchecked);
    }
    darkThemeBox->addWidget(darkThemeSwitch);
    generalBloc->addSeparator();

    // Monochrome icons activation
    QBoxLayout *monochromeIconsBox = generalBloc->addLayout(QBoxLayout::Direction::LeftToRight);

    QLabel *monochromeLabel = new QLabel(tr("Activate monochrome icons"), this);
    monochromeIconsBox->addWidget(monochromeLabel);
    monochromeIconsBox->addStretch();

    CustomSwitch *monochromeSwitch = new CustomSwitch(this);
    monochromeSwitch->setLayoutDirection(Qt::RightToLeft);
    monochromeSwitch->setAttribute(Qt::WA_MacShowFocusRect, false);
    monochromeSwitch->setCheckState(cfg.monoIcons() ? Qt::Checked : Qt::Unchecked);
    monochromeIconsBox->addWidget(monochromeSwitch);
    generalBloc->addSeparator();

    // Launch kDrive at startup
    QBoxLayout *launchAtStartupBox = generalBloc->addLayout(QBoxLayout::Direction::LeftToRight);

    QLabel *launchAtStartupLabel = new QLabel(tr("Launch kDrive at startup"), this);
    launchAtStartupBox->addWidget(launchAtStartupLabel);
    launchAtStartupBox->addStretch();

    CustomSwitch *launchAtStartupSwitch = new CustomSwitch(this);
    launchAtStartupSwitch->setLayoutDirection(Qt::RightToLeft);
    launchAtStartupSwitch->setAttribute(Qt::WA_MacShowFocusRect, false);
    bool hasSystemLauchAtStartup = OCC::Utility::hasSystemLaunchOnStartup(OCC::Theme::instance()->appName());
    if (hasSystemLauchAtStartup) {
        // Cannot disable autostart because system-wide autostart is enabled
        launchAtStartupSwitch->setCheckState(Qt::Checked);
        launchAtStartupSwitch->setDisabled(true);
    }
    else {
        bool hasLaunchAtStartup = OCC::Utility::hasLaunchOnStartup(OCC::Theme::instance()->appName());
        launchAtStartupSwitch->setCheckState(hasLaunchAtStartup ? Qt::Checked : Qt::Unchecked);
    }
    launchAtStartupBox->addWidget(launchAtStartupSwitch);

    //
    // Advanced bloc
    //
    QLabel *advancedLabel = new QLabel(tr("Advanced"), this);
    advancedLabel->setObjectName("blocLabel");
    vBox->addWidget(advancedLabel);

    PreferencesBlocWidget *advancedBloc = new PreferencesBlocWidget(this);
    vBox->addWidget(advancedBloc);

    // Files to exclude
    QVBoxLayout *filesToExcludeVBox = nullptr;
    ClickableWidget *filesToExcludeWidget = advancedBloc->addActionWidget(&filesToExcludeVBox);

    QLabel *filesToExcludeLabel = new QLabel(tr("Files to exclude"), this);
    filesToExcludeVBox->addWidget(filesToExcludeLabel);
    advancedBloc->addSeparator();

    // Proxy server
    QVBoxLayout *proxyServerVBox = nullptr;
    ClickableWidget *proxyServerWidget = advancedBloc->addActionWidget(&proxyServerVBox);

    QLabel *proxyServerLabel = new QLabel(tr("Proxy server"), this);
    proxyServerVBox->addWidget(proxyServerLabel);
    advancedBloc->addSeparator();

    // Bandwidth
    QVBoxLayout *bandwidthVBox = nullptr;
    ClickableWidget *bandwidthWidget = advancedBloc->addActionWidget(&bandwidthVBox);

    QLabel *bandwidthLabel = new QLabel(tr("Bandwidth"), this);
    bandwidthVBox->addWidget(bandwidthLabel);
    bandwidthVBox->addStretch();

    vBox->addStretch();

    connect(folderConfirmationSwitch, &CustomSwitch::clicked, this, &PreferencesWidget::onFolderConfirmationSwitchClicked);
    connect(_folderConfirmationAmountLineEdit, &QLineEdit::editingFinished, this, &PreferencesWidget::onFolderConfirmationAmountEditingFinished);
    connect(darkThemeSwitch, &CustomSwitch::clicked,this, &PreferencesWidget::onDarkThemeSwitchClicked);
    connect(monochromeSwitch, &CustomSwitch::clicked, this, &PreferencesWidget::onMonochromeSwitchClicked);
    connect(launchAtStartupSwitch, &CustomSwitch::clicked, this, &PreferencesWidget::onLaunchAtStartupSwitchClicked);
    connect(filesToExcludeWidget, &ClickableWidget::clicked, this, &PreferencesWidget::onFilesToExcludeWidgetClicked);
    connect(proxyServerWidget, &ClickableWidget::clicked, this, &PreferencesWidget::onProxyServerWidgetClicked);
    connect(bandwidthWidget, &ClickableWidget::clicked, this, &PreferencesWidget::onBandwidthWidgetClicked);
}

void PreferencesWidget::onFolderConfirmationSwitchClicked(bool checked)
{
    OCC::ConfigFile cfg;
    auto folderLimit = cfg.newBigFolderSizeLimit();
    cfg.setNewBigFolderSizeLimit(checked, folderLimit.second);
    _folderConfirmationAmountLineEdit->setEnabled(checked);
}

void PreferencesWidget::onFolderConfirmationAmountEditingFinished()
{
    long long lValue = _folderConfirmationAmountLineEdit->text().toLongLong();
    OCC::ConfigFile cfg;
    auto folderLimit = cfg.newBigFolderSizeLimit();
    cfg.setNewBigFolderSizeLimit(folderLimit.first, lValue);
}

void PreferencesWidget::onDarkThemeSwitchClicked(bool checked)
{
    emit setStyle(checked);
}

void PreferencesWidget::onMonochromeSwitchClicked(bool checked)
{
    OCC::ConfigFile cfg;
    cfg.setMonoIcons(checked);
    OCC::Theme::instance()->setSystrayUseMonoIcons(checked);
}

void PreferencesWidget::onLaunchAtStartupSwitchClicked(bool checked)
{
    OCC::Theme *theme = OCC::Theme::instance();
    OCC::Utility::setLaunchOnStartup(theme->appName(), theme->appNameGUI(), checked);
}

void PreferencesWidget::onFilesToExcludeWidgetClicked()
{
    FileExclusionDialog *dialog = new FileExclusionDialog(this);
    dialog->exec();
}

void PreferencesWidget::onProxyServerWidgetClicked()
{
    ProxyServerDialog *dialog = new ProxyServerDialog(this);
    dialog->exec();
}

void PreferencesWidget::onBandwidthWidgetClicked()
{

}

}
