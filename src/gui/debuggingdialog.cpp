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

#include "debuggingdialog.h"
#include "custommessagebox.h"
#include "configfile.h"
#include "logger.h"

#include <map>

#include <QBoxLayout>
#include <QLabel>

namespace KDC {

static const int boxHMargin = 40;
static const int boxHSpacing = 10;
static const int titleBoxVMargin = 25;
static const int recordDebuggingBoxVMargin = 20;
static const int debugLevelLabelBoxVMargin = 10;
static const int debugLevelSelectBoxVMargin = 20;
static const std::chrono::hours defaultExpireDuration(24);

std::map<DebuggingDialog::DebugLevel, std::pair<int, QString>> DebuggingDialog::_debugLevelMap = {
    { DebugLevel::Info, { 0, QString(tr("Info")) } },
    { DebugLevel::Debug, { 1, QString(tr("Debug")) } },
    { DebugLevel::Warning, { 2, QString(tr("Warning")) } },
    { DebugLevel::Critical, { 3, QString(tr("Critical")) } },
    { DebugLevel::Fatal, { 4, QString(tr("Fatal")) } }
};

DebuggingDialog::DebuggingDialog(QWidget *parent)
    : CustomDialog(true, false, parent)
    , _recordDebuggingSwitch(nullptr)
    , _debugLevelComboBox(nullptr)
    , _saveButton(nullptr)
    , _recordDebugging(false)
    , _minLogLevel(DebugLevel::Info)
    , _deleteLogs(false)
{
    initUI();

    OCC::ConfigFile cfg;
    _recordDebugging = cfg.automaticLogDir();
    _minLogLevel = (DebugLevel) cfg.minLogLevel();
    _deleteLogs = (bool) cfg.automaticDeleteOldLogsAge();

    updateUI();
}

void DebuggingDialog::initUI()
{
    QVBoxLayout *mainLayout = this->mainLayout();

    // Title
    QLabel *titleLabel = new QLabel(this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setContentsMargins(boxHMargin, 0, boxHMargin, titleBoxVMargin);
    titleLabel->setText(tr("Record debugging information"));
    mainLayout->addWidget(titleLabel);

    // Record debugging information
    QHBoxLayout *recordDebuggingHBox = new QHBoxLayout();
    recordDebuggingHBox->setContentsMargins(boxHMargin, 0, boxHMargin, recordDebuggingBoxVMargin);
    mainLayout->addLayout(recordDebuggingHBox);

    QLabel *recordDebuggingLabel = new QLabel(this);
    recordDebuggingLabel->setObjectName("boldTextLabel");
    recordDebuggingLabel->setText(tr("Activate the recording of information in a temporary folder"));
    recordDebuggingHBox->addWidget(recordDebuggingLabel);
    recordDebuggingHBox->addStretch();

    _recordDebuggingSwitch = new CustomSwitch(this);
    _recordDebuggingSwitch->setLayoutDirection(Qt::RightToLeft);
    _recordDebuggingSwitch->setAttribute(Qt::WA_MacShowFocusRect, false);
    recordDebuggingHBox->addWidget(_recordDebuggingSwitch);

    // Minimum debug level
    QLabel *debugLevelLabel = new QLabel(this);
    debugLevelLabel->setObjectName("boldTextLabel");
    debugLevelLabel->setContentsMargins(boxHMargin, 0, boxHMargin, debugLevelLabelBoxVMargin);
    debugLevelLabel->setText(tr("Minimum trace level"));
    mainLayout->addWidget(debugLevelLabel);

    QHBoxLayout *debugLevelHBox = new QHBoxLayout();
    debugLevelHBox->setContentsMargins(boxHMargin, 0, boxHMargin, debugLevelSelectBoxVMargin);
    mainLayout->addLayout(debugLevelHBox);

    _debugLevelComboBox = new CustomComboBox(this);
    _debugLevelComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    _debugLevelComboBox->setAttribute(Qt::WA_MacShowFocusRect, false);
    for (auto const &debugLevelElt : _debugLevelMap) {
        _debugLevelComboBox->insertItem(debugLevelElt.second.first, debugLevelElt.second.second, debugLevelElt.first);
    }
    debugLevelHBox->addWidget(_debugLevelComboBox);
    debugLevelHBox->addStretch();

    // Delete logs
    QHBoxLayout *deleteLogsHBox = new QHBoxLayout();
    deleteLogsHBox->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    mainLayout->addLayout(deleteLogsHBox);

    _deleteLogsCheckBox = new CustomCheckBox(this);
    _deleteLogsCheckBox->setObjectName("deleteLogsCheckBox");
    _deleteLogsCheckBox->setText(tr("Delete logs older than %1 hours")
                                 .arg(defaultExpireDuration.count()));
    deleteLogsHBox->addWidget(_deleteLogsCheckBox);

    mainLayout->addStretch();

    // Add dialog buttons
    QHBoxLayout *buttonsHBox = new QHBoxLayout();
    buttonsHBox->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    buttonsHBox->setSpacing(boxHSpacing);
    mainLayout->addLayout(buttonsHBox);

    _saveButton = new QPushButton(this);
    _saveButton->setObjectName("defaultbutton");
    _saveButton->setFlat(true);
    _saveButton->setText(tr("SAVE"));
    _saveButton->setEnabled(false);
    buttonsHBox->addWidget(_saveButton);

    QPushButton *cancelButton = new QPushButton(this);
    cancelButton->setFlat(true);
    cancelButton->setText(tr("CANCEL"));
    buttonsHBox->addWidget(cancelButton);
    buttonsHBox->addStretch();

    connect(_recordDebuggingSwitch, &CustomSwitch::clicked, this, &DebuggingDialog::onRecordDebuggingSwitchClicked);
    connect(_debugLevelComboBox, QOverload<int>::of(&QComboBox::activated), this, &DebuggingDialog::onDebugLevelComboBoxActivated);
    connect(_deleteLogsCheckBox, &CustomCheckBox::clicked, this, &DebuggingDialog::onDeleteLogsCheckBoxClicked);
    connect(_saveButton, &QPushButton::clicked, this, &DebuggingDialog::onSaveButtonTriggered);
    connect(cancelButton, &QPushButton::clicked, this, &DebuggingDialog::onExit);
    connect(this, &CustomDialog::exit, this, &DebuggingDialog::onExit);
}

void DebuggingDialog::updateUI()
{
    _recordDebuggingSwitch->setCheckState(_recordDebugging ? Qt::Checked : Qt::Unchecked);

    _debugLevelComboBox->setEnabled(_recordDebugging);
    _debugLevelComboBox->setCurrentIndex(_recordDebugging ? _minLogLevel : -1);

    _deleteLogsCheckBox->setEnabled(_recordDebugging);
    _deleteLogsCheckBox->setChecked(_recordDebugging ? _deleteLogs : false);
}

void DebuggingDialog::setNeedToSave(bool value)
{
    _needToSave = value;
    _saveButton->setEnabled(value);
}

void DebuggingDialog::onRecordDebuggingSwitchClicked(bool checked)
{
    _recordDebugging = checked;
    updateUI();
    setNeedToSave(true);
}

void DebuggingDialog::onDebugLevelComboBoxActivated(int index)
{
    _minLogLevel = qvariant_cast<DebugLevel>(_debugLevelComboBox->itemData(index));
    setNeedToSave(true);
}

void DebuggingDialog::onDeleteLogsCheckBoxClicked(bool checked)
{
    _deleteLogs = checked;
    setNeedToSave(true);
}

void DebuggingDialog::onExit()
{
    if (_needToSave) {
        CustomMessageBox *msgBox = new CustomMessageBox(
                    QMessageBox::Question,
                    tr("Do you want to save your modifications?"),
                    QMessageBox::Yes | QMessageBox::No, this);
        msgBox->setDefaultButton(QMessageBox::Yes);
        if (msgBox->exec() == QMessageBox::Yes) {
            onSaveButtonTriggered();
        }
        else {
            reject();
        }
    }
    else {
        reject();
    }
}

void DebuggingDialog::onSaveButtonTriggered(bool checked)
{
    Q_UNUSED(checked)

    OCC::ConfigFile cfg;
    auto logger = OCC::Logger::instance();

    // Record debugging information
    cfg.setAutomaticLogDir(_recordDebugging);
    if (_recordDebugging) {
        if (!logger->isLoggingToFile()) {
            logger->setupTemporaryFolderLogDir();
            logger->enterNextLogFile();
        }
    } else {
        logger->disableTemporaryFolderLogDir();
    }

    // Minimum debug level
    cfg.setMinLogLevel(_minLogLevel);
    logger->setMinLogLevel(cfg.minLogLevel());

    // Delete logs
    if (_recordDebugging && _deleteLogs) {
        cfg.setAutomaticDeleteOldLogsAge(defaultExpireDuration);
        logger->setLogExpire(defaultExpireDuration);
    } else {
        cfg.setAutomaticDeleteOldLogsAge({});
        logger->setLogExpire(std::chrono::hours(0));
    }

    accept();
}

}
