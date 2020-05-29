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
#include "configfile.h"

#include <map>

#include <QBoxLayout>
#include <QLabel>
#include <QMessageBox>

namespace KDC {

static const int boxHMargin= 40;
static const int boxHSpacing = 10;
static const int titleBoxVMargin = 25;

std::map<DebuggingDialog::DebugLevel, std::pair<int, QString>> DebuggingDialog::_debugLevelMap = {
    { DebugLevel::Info, { 0, QString(tr("Info")) } },
    { DebugLevel::Debug, { 1, QString(tr("Debug")) } },
    { DebugLevel::Warning, { 2, QString(tr("Warning")) } },
    { DebugLevel::Critical, { 3, QString(tr("Critical")) } },
    { DebugLevel::Fatal, { 4, QString(tr("Fatal")) } }
};

DebuggingDialog::DebuggingDialog(QWidget *parent)
    : CustomDialog(true, parent)
    , _recordDebuggingSwitch(nullptr)
    , _debugLevelComboBox(nullptr)
    , _saveButton(nullptr)
{
    initUI();
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
    recordDebuggingHBox->setContentsMargins(0, 0, 0, 0);
    mainLayout->addLayout(recordDebuggingHBox);

    QLabel *recordDebuggingLabel = new QLabel(tr("Activate the recording of information in a temporary folder"), this);
    recordDebuggingLabel->setObjectName("boldtextLabel");
    recordDebuggingHBox->addWidget(recordDebuggingLabel);
    recordDebuggingHBox->addStretch();

    _recordDebuggingSwitch = new CustomSwitch(this);
    _recordDebuggingSwitch->setLayoutDirection(Qt::RightToLeft);
    _recordDebuggingSwitch->setAttribute(Qt::WA_MacShowFocusRect, false);
    recordDebuggingHBox->addWidget(_recordDebuggingSwitch);

    // Minimum trace level
    QLabel *debugLevelLabel = new QLabel(tr("Minimum trace level"), this);
    debugLevelLabel->setObjectName("boldtextLabel");
    mainLayout->addWidget(debugLevelLabel);

    QHBoxLayout *debugLevelHBox = new QHBoxLayout();
    debugLevelHBox->setContentsMargins(0, 0, 0, 0);
    mainLayout->addLayout(debugLevelHBox);

    _debugLevelComboBox = new CustomComboBox(this);
    _debugLevelComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    _debugLevelComboBox->setAttribute(Qt::WA_MacShowFocusRect, false);

    for (auto const &debugLevelElt : _debugLevelMap) {
        _debugLevelComboBox->insertItem(debugLevelElt.second.first, debugLevelElt.second.second, debugLevelElt.first);
    }
    debugLevelHBox->addWidget(_debugLevelComboBox);
    debugLevelHBox->addStretch();




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

    connect(_saveButton, &QPushButton::clicked, this, &DebuggingDialog::onSaveButtonTriggered);
    connect(cancelButton, &QPushButton::clicked, this, &DebuggingDialog::onExit);
    connect(this, &CustomDialog::exit, this, &DebuggingDialog::onExit);
}

void DebuggingDialog::updateUI()
{
    OCC::ConfigFile cfg;

    _recordDebuggingSwitch->setCheckState(cfg.automaticLogDir() ? Qt::Checked : Qt::Unchecked);

}

void DebuggingDialog::setNeedToSave(bool value)
{
    _needToSave = value;
    _saveButton->setEnabled(value);
}

void DebuggingDialog::onExit()
{
    if (_needToSave) {
        QMessageBox msgBox(QMessageBox::Question, QString(),
                           tr("Do you want to save your modifications?"),
                           QMessageBox::Yes | QMessageBox::No, this);
        msgBox.setWindowModality(Qt::WindowModal);
        msgBox.setDefaultButton(QMessageBox::Yes);
        if (msgBox.exec() == QMessageBox::Yes) {
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




    accept();
}

}
