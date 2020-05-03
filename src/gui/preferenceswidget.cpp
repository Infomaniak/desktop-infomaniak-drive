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
#include "configfile.h"
#include "guiutility.h"
#include "common/utility.h"
#include "theme.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QIntValidator>

namespace KDC {

static const int boxHMargin= 20;
static const int boxVMargin = 20;
static const int boxSpacing = 12;
static const int textHSpacing = 10;
static const int textVSpacing = 5;
static const int actionIconSize = 16;
static const QSize amountLineEditSize = QSize(85, 40);

PreferencesWidget::PreferencesWidget(QWidget *parent)
    : QWidget(parent)
    , _actionColor(QColor())
    , _filesToExcludeIconLabel(nullptr)
    , _proxyServerIconLabel(nullptr)
    , _bandwidthIconLabel(nullptr)
    , _folderConfirmationAmountLineEdit(nullptr)
{
    setContentsMargins(0, 0, 0, 0);

    OCC::ConfigFile cfg;

    QVBoxLayout *vbox = new QVBoxLayout();
    vbox->setContentsMargins(boxHMargin, boxVMargin, boxHMargin, boxVMargin);
    vbox->setSpacing(boxSpacing);
    setLayout(vbox);

    //
    // General bloc
    //
    QLabel *generalLabel = new QLabel(tr("General"), this);
    generalLabel->setObjectName("generalLabel");
    vbox->addWidget(generalLabel);

    PreferencesBlocWidget *generalBloc = new PreferencesBlocWidget(this);
    vbox->addWidget(generalBloc);

    // Folder synchronization confirmation
    QBoxLayout *folderConfirmationBox = generalBloc->addLayout(QBoxLayout::Direction::TopToBottom);
    folderConfirmationBox->setSpacing(textVSpacing);

    QHBoxLayout *folderConfirmation1HBox = new QHBoxLayout();
    folderConfirmation1HBox->setContentsMargins(0, 0, 0, 0);
    folderConfirmationBox->addLayout(folderConfirmation1HBox);

    QLabel *folderConfirmationLabel = new QLabel(tr("Ask for confirmation before synchronizing files greater than"), this);
    folderConfirmation1HBox->addWidget(folderConfirmationLabel);
    folderConfirmation1HBox->addStretch();

    QCheckBox *folderConfirmationCheckBox = new QCheckBox(this);
    auto folderLimit = cfg.newBigFolderSizeLimit();
    folderConfirmationCheckBox->setCheckState(folderLimit.first ? Qt::Checked : Qt::Unchecked);
    folderConfirmation1HBox->addWidget(folderConfirmationCheckBox);

    QHBoxLayout *folderConfirmation2HBox = new QHBoxLayout();
    folderConfirmation2HBox->setContentsMargins(0, 0, 0, 0);
    folderConfirmationBox->addLayout(folderConfirmation2HBox);
    folderConfirmation2HBox->setSpacing(textHSpacing);

    _folderConfirmationAmountLineEdit = new QLineEdit(this);
    _folderConfirmationAmountLineEdit->setEnabled(folderLimit.first);
    _folderConfirmationAmountLineEdit->setText(QString::number(folderLimit.second));
    _folderConfirmationAmountLineEdit->setValidator(new QIntValidator(0, 999999, this));
    _folderConfirmationAmountLineEdit->setMinimumSize(amountLineEditSize);
    _folderConfirmationAmountLineEdit->setMaximumSize(amountLineEditSize);
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

    QCheckBox *darkThemeCheckBox = new QCheckBox(this);
    if (OCC::Utility::isMac()) {
        bool darkSystray = OCC::Utility::hasDarkSystray();
        darkThemeCheckBox->setCheckState(darkSystray ? Qt::Checked : Qt::Unchecked);
        darkThemeCheckBox->setDisabled(true);
    }
    else {
        darkThemeCheckBox->setCheckState(cfg.darkTheme() ? Qt::Checked : Qt::Unchecked);
    }
    darkThemeBox->addWidget(darkThemeCheckBox);
    generalBloc->addSeparator();

    // Monochrome icons activation
    QBoxLayout *monochromeIconsBox = generalBloc->addLayout(QBoxLayout::Direction::LeftToRight);

    QLabel *monochromeLabel = new QLabel(tr("Activate monochrome icons"), this);
    monochromeIconsBox->addWidget(monochromeLabel);
    monochromeIconsBox->addStretch();

    QCheckBox *monochromeCheckBox = new QCheckBox(this);
    monochromeCheckBox->setCheckState(cfg.monoIcons() ? Qt::Checked : Qt::Unchecked);
    monochromeIconsBox->addWidget(monochromeCheckBox);
    generalBloc->addSeparator();

    // Launch kDrive at startup
    QBoxLayout *launchAtStartupBox = generalBloc->addLayout(QBoxLayout::Direction::LeftToRight);

    QLabel *launchAtStartupLabel = new QLabel(tr("Launch kDrive at startup"), this);
    launchAtStartupBox->addWidget(launchAtStartupLabel);
    launchAtStartupBox->addStretch();

    QCheckBox *launchAtStartupCheckBox = new QCheckBox(this);
    bool hasSystemLauchAtStartup = OCC::Utility::hasSystemLaunchOnStartup(OCC::Theme::instance()->appName());
    if (hasSystemLauchAtStartup) {
        // Cannot disable autostart because system-wide autostart is enabled
        launchAtStartupCheckBox->setCheckState(Qt::Checked);
        launchAtStartupCheckBox->setDisabled(true);
    }
    else {
        bool hasLaunchAtStartup = OCC::Utility::hasLaunchOnStartup(OCC::Theme::instance()->appName());
        launchAtStartupCheckBox->setCheckState(hasLaunchAtStartup ? Qt::Checked : Qt::Unchecked);
    }
    launchAtStartupBox->addWidget(launchAtStartupCheckBox);

    //
    // Advanced bloc
    //
    QLabel *advancedLabel = new QLabel(tr("Advanced"), this);
    advancedLabel->setObjectName("advancedLabel");
    vbox->addWidget(advancedLabel);

    PreferencesBlocWidget *advancedBloc = new PreferencesBlocWidget(this);
    vbox->addWidget(advancedBloc);

    // Files to exclude
    ClickableWidget *filesToExcludeWidget = advancedBloc->addWidget();
    QHBoxLayout *filesToExcludeHBox = qobject_cast<QHBoxLayout *>(filesToExcludeWidget->layout());

    QLabel *filesToExcludeLabel = new QLabel(tr("Files to exclude"), this);
    filesToExcludeHBox->addWidget(filesToExcludeLabel);
    filesToExcludeHBox->addStretch();

    _filesToExcludeIconLabel = new QLabel(this);
    filesToExcludeHBox->addWidget(_filesToExcludeIconLabel);
    advancedBloc->addSeparator();

    // Proxy server
    ClickableWidget *proxyServerWidget = advancedBloc->addWidget();
    QHBoxLayout *proxyServerHBox = qobject_cast<QHBoxLayout *>(proxyServerWidget->layout());

    QLabel *proxyServerLabel = new QLabel(tr("Proxy server"), this);
    proxyServerHBox->addWidget(proxyServerLabel);
    proxyServerHBox->addStretch();

    _proxyServerIconLabel = new QLabel(this);
    proxyServerHBox->addWidget(_proxyServerIconLabel);
    advancedBloc->addSeparator();

    // Bandwidth
    ClickableWidget *bandwidthWidget = advancedBloc->addWidget();
    QHBoxLayout *bandwidthHBox = qobject_cast<QHBoxLayout *>(bandwidthWidget->layout());

    QLabel *bandwidthLabel = new QLabel(tr("Bandwidth"), this);
    bandwidthHBox->addWidget(bandwidthLabel);
    bandwidthHBox->addStretch();

    _bandwidthIconLabel = new QLabel(this);
    bandwidthHBox->addWidget(_bandwidthIconLabel);

    vbox->addStretch();

    connect(this, &PreferencesWidget::actionColorChanged, this, &PreferencesWidget::onActionColorChanged);
    connect(folderConfirmationCheckBox, &QCheckBox::stateChanged,
            this, &PreferencesWidget::onFolderConfirmationCheckBoxStateChanged);
    connect(_folderConfirmationAmountLineEdit, &QLineEdit::editingFinished,
            this, &PreferencesWidget::onFolderConfirmationAmountEditingFinished);
    connect(darkThemeCheckBox, &QCheckBox::stateChanged,
            this, &PreferencesWidget::onDarkThemeCheckBoxStateChanged);
    connect(monochromeCheckBox, &QCheckBox::stateChanged,
            this, &PreferencesWidget::onMonochromeCheckBoxStateChanged);
    connect(launchAtStartupCheckBox, &QCheckBox::stateChanged,
            this, &PreferencesWidget::onLaunchAtStartupCheckBoxStateChanged);
    connect(filesToExcludeWidget, &ClickableWidget::clicked, this, &PreferencesWidget::onFilesToExcludeWidgetClicked);
    connect(proxyServerWidget, &ClickableWidget::clicked, this, &PreferencesWidget::onProxyServerWidgetClicked);
    connect(bandwidthWidget, &ClickableWidget::clicked, this, &PreferencesWidget::onBandwidthWidgetClicked);
}

void PreferencesWidget::onActionColorChanged()
{
    _filesToExcludeIconLabel->setPixmap(
                OCC::Utility::getIconWithColor(":/client/resources/icons/actions/chevron-right.svg", _actionColor)
                .pixmap(actionIconSize));

    _proxyServerIconLabel->setPixmap(
                OCC::Utility::getIconWithColor(":/client/resources/icons/actions/chevron-right.svg", _actionColor)
                .pixmap(actionIconSize));

    _bandwidthIconLabel->setPixmap(
                OCC::Utility::getIconWithColor(":/client/resources/icons/actions/chevron-right.svg", _actionColor)
                .pixmap(actionIconSize));
}

void PreferencesWidget::onFolderConfirmationCheckBoxStateChanged(int state)
{
    OCC::ConfigFile cfg;
    auto folderLimit = cfg.newBigFolderSizeLimit();
    cfg.setNewBigFolderSizeLimit(state == Qt::Checked ? true : false, folderLimit.second);
    _folderConfirmationAmountLineEdit->setEnabled(state == Qt::Checked ? true : false);
}

void PreferencesWidget::onFolderConfirmationAmountEditingFinished()
{
    long long lValue = _folderConfirmationAmountLineEdit->text().toLongLong();
    OCC::ConfigFile cfg;
    auto folderLimit = cfg.newBigFolderSizeLimit();
    cfg.setNewBigFolderSizeLimit(folderLimit.first, lValue);
}

void PreferencesWidget::onDarkThemeCheckBoxStateChanged(int state)
{
    OCC::ConfigFile cfg;
    cfg.setDarkTheme(state == Qt::Checked ? true : false);
    OCC::Utility::setStyle(qApp, state == Qt::Checked ? true : false);
}

void PreferencesWidget::onMonochromeCheckBoxStateChanged(int state)
{
    OCC::ConfigFile cfg;
    cfg.setMonoIcons(state == Qt::Checked ? true : false);
    OCC::Theme::instance()->setSystrayUseMonoIcons(state == Qt::Checked ? true : false);
}

void PreferencesWidget::onLaunchAtStartupCheckBoxStateChanged(int state)
{
    OCC::Theme *theme = OCC::Theme::instance();
    OCC::Utility::setLaunchOnStartup(theme->appName(), theme->appNameGUI(),
                                     state == Qt::Checked ? true : false);
}

void PreferencesWidget::onFilesToExcludeWidgetClicked()
{

}

void PreferencesWidget::onProxyServerWidgetClicked()
{

}

void PreferencesWidget::onBandwidthWidgetClicked()
{

}

}
