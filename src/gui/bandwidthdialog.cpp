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

#include "bandwidthdialog.h"
#include "custommessagebox.h"
#include "configfile.h"
#include "folderman.h"

#include <QButtonGroup>
#include <QIntValidator>
#include <QLabel>
#include <QLoggingCategory>

namespace KDC {

static const int boxHMargin = 40;
static const int boxHSpacing = 10;
static const int titleBoxVMargin = 25;
static const int subtitleLabelVMargin = 10;
static const int noLimitButtonVMargin = 15;
static const int valueLimitButtonVMargin = 40;
static const int valueEditSize = 80;

Q_LOGGING_CATEGORY(lcBandwidthDialog, "bandwidthdialog", QtInfoMsg)

BandwidthDialog::BandwidthDialog(QWidget *parent)
    : CustomDialog(true, parent)
    , _useDownloadLimit(0)
    , _downloadLimit(0)
    , _useUploadLimit(0)
    , _uploadLimit(0)
    , _downloadNoLimitButton(nullptr)
    , _downloadAutoLimitButton(nullptr)
    , _downloadValueLimitLineEdit(nullptr)
    , _uploadNoLimitButton(nullptr)
    , _uploadAutoLimitButton(nullptr)
    , _uploadValueLimitButton(nullptr)
    , _uploadValueLimitLineEdit(nullptr)
    , _saveButton(nullptr)
    , _needToSave(false)
{
    initUI();

    // Load saved configuration
    OCC::ConfigFile cfg;
    if (cfg.exists()) {
        _useDownloadLimit = cfg.useDownloadLimit();
        _downloadLimit = cfg.downloadLimit();
        _useUploadLimit = cfg.useUploadLimit();
        _uploadLimit = cfg.uploadLimit();
    }

    updateUI();
}

void BandwidthDialog::initUI()
{
    QVBoxLayout *mainLayout = this->mainLayout();

    // Title
    QLabel *titleLabel = new QLabel(this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setContentsMargins(boxHMargin, 0, boxHMargin, titleBoxVMargin);
    titleLabel->setText(tr("Bandwidth"));
    mainLayout->addWidget(titleLabel);

    // Download bandwidth
    QLabel *downloadLabel = new QLabel(this);
    downloadLabel->setObjectName("subtitleLabel");
    downloadLabel->setContentsMargins(boxHMargin, 0, boxHMargin, subtitleLabelVMargin);
    downloadLabel->setText(tr("Download bandwidth"));
    mainLayout->addWidget(downloadLabel);

    QButtonGroup *downloadButtonGroup = new QButtonGroup(this);
    downloadButtonGroup->setExclusive(true);

    QHBoxLayout *downloadNoLimitHBox = new QHBoxLayout();
    downloadNoLimitHBox->setContentsMargins(boxHMargin, 0, boxHMargin, noLimitButtonVMargin);
    mainLayout->addLayout(downloadNoLimitHBox);

    _downloadNoLimitButton = new CustomRadioButton(this);
    _downloadNoLimitButton->setText(tr("No limit"));
    _downloadNoLimitButton->setAttribute(Qt::WA_MacShowFocusRect, false);
    downloadNoLimitHBox->addWidget(_downloadNoLimitButton);
    downloadNoLimitHBox->addStretch();
    downloadButtonGroup->addButton(_downloadNoLimitButton);

    QHBoxLayout *downloadAutoLimitHBox = new QHBoxLayout();
    downloadAutoLimitHBox->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    mainLayout->addLayout(downloadAutoLimitHBox);

    _downloadAutoLimitButton = new CustomRadioButton(this);
    _downloadAutoLimitButton->setText(tr("Automatically limit"));
    _downloadAutoLimitButton->setAttribute(Qt::WA_MacShowFocusRect, false);
    downloadAutoLimitHBox->addWidget(_downloadAutoLimitButton);
    downloadAutoLimitHBox->addStretch();
    downloadButtonGroup->addButton(_downloadAutoLimitButton);

    QHBoxLayout *downloadValueLimitHBox = new QHBoxLayout();
    downloadValueLimitHBox->setContentsMargins(boxHMargin, 0, boxHMargin, valueLimitButtonVMargin);
    downloadValueLimitHBox->setSpacing(boxHSpacing);
    mainLayout->addLayout(downloadValueLimitHBox);

    _downloadValueLimitButton = new CustomRadioButton(this);
    _downloadValueLimitButton->setText(tr("Limit to"));
    _downloadValueLimitButton->setAttribute(Qt::WA_MacShowFocusRect, false);
    downloadValueLimitHBox->addWidget(_downloadValueLimitButton);
    downloadButtonGroup->addButton(_downloadValueLimitButton);

    _downloadValueLimitLineEdit = new QLineEdit(this);
    _downloadValueLimitLineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    _downloadValueLimitLineEdit->setValidator(new QIntValidator(0, 999999, this));
    _downloadValueLimitLineEdit->setMinimumWidth(valueEditSize);
    _downloadValueLimitLineEdit->setMaximumWidth(valueEditSize);
    downloadValueLimitHBox->addWidget(_downloadValueLimitLineEdit);

    QLabel *downloadValueLabel = new QLabel(this);
    downloadValueLabel->setObjectName("textLabel");
    downloadValueLabel->setText("Ko/s");
    downloadValueLimitHBox->addWidget(downloadValueLabel);
    downloadValueLimitHBox->addStretch();

    // Upload bandwidth
    QLabel *uploadLabel = new QLabel(this);
    uploadLabel->setObjectName("subtitleLabel");
    uploadLabel->setContentsMargins(boxHMargin, 0, boxHMargin, subtitleLabelVMargin);
    uploadLabel->setText(tr("Upload bandwidth"));
    mainLayout->addWidget(uploadLabel);

    QButtonGroup *uploadButtonGroup = new QButtonGroup(this);
    uploadButtonGroup->setExclusive(true);

    QHBoxLayout *uploadNoLimitHBox = new QHBoxLayout();
    uploadNoLimitHBox->setContentsMargins(boxHMargin, 0, boxHMargin, noLimitButtonVMargin);
    mainLayout->addLayout(uploadNoLimitHBox);

    _uploadNoLimitButton = new CustomRadioButton(this);
    _uploadNoLimitButton->setText(tr("No limit"));
    _uploadNoLimitButton->setAttribute(Qt::WA_MacShowFocusRect, false);
    uploadNoLimitHBox->addWidget(_uploadNoLimitButton);
    uploadNoLimitHBox->addStretch();
    uploadButtonGroup->addButton(_uploadNoLimitButton);

    QHBoxLayout *uploadAutoLimitHBox = new QHBoxLayout();
    uploadAutoLimitHBox->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    mainLayout->addLayout(uploadAutoLimitHBox);

    _uploadAutoLimitButton = new CustomRadioButton(this);
    _uploadAutoLimitButton->setText(tr("Automatically limit"));
    _uploadAutoLimitButton->setAttribute(Qt::WA_MacShowFocusRect, false);
    uploadAutoLimitHBox->addWidget(_uploadAutoLimitButton);
    uploadAutoLimitHBox->addStretch();
    uploadButtonGroup->addButton(_uploadAutoLimitButton);

    QHBoxLayout *uploadValueLimitHBox = new QHBoxLayout();
    uploadValueLimitHBox->setContentsMargins(boxHMargin, 0, boxHMargin, valueLimitButtonVMargin);
    uploadValueLimitHBox->setSpacing(boxHSpacing);
    mainLayout->addLayout(uploadValueLimitHBox);

    _uploadValueLimitButton = new CustomRadioButton(this);
    _uploadValueLimitButton->setText(tr("Limit to"));
    _uploadValueLimitButton->setAttribute(Qt::WA_MacShowFocusRect, false);
    uploadValueLimitHBox->addWidget(_uploadValueLimitButton);
    uploadButtonGroup->addButton(_uploadValueLimitButton);

    _uploadValueLimitLineEdit = new QLineEdit(this);
    _uploadValueLimitLineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    _uploadValueLimitLineEdit->setValidator(new QIntValidator(0, 999999, this));
    _uploadValueLimitLineEdit->setMinimumWidth(valueEditSize);
    _uploadValueLimitLineEdit->setMaximumWidth(valueEditSize);
    uploadValueLimitHBox->addWidget(_uploadValueLimitLineEdit);

    QLabel *uploadValueLabel = new QLabel(this);
    uploadValueLabel->setObjectName("textLabel");
    uploadValueLabel->setText("Ko/s");
    uploadValueLimitHBox->addWidget(uploadValueLabel);
    uploadValueLimitHBox->addStretch();

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
    cancelButton->setObjectName("nondefaultbutton");
    cancelButton->setFlat(true);
    cancelButton->setText(tr("CANCEL"));
    buttonsHBox->addWidget(cancelButton);
    buttonsHBox->addStretch();

    connect(_saveButton, &QPushButton::clicked, this, &BandwidthDialog::onSaveButtonTriggered);
    connect(cancelButton, &QPushButton::clicked, this, &BandwidthDialog::onExit);
    connect(this, &CustomDialog::exit, this, &BandwidthDialog::onExit);
    connect(_downloadNoLimitButton, &CustomRadioButton::toggled, this, &BandwidthDialog::onDownloadNoLimitButtonToggled);
    connect(_downloadAutoLimitButton, &CustomRadioButton::toggled, this, &BandwidthDialog::onDownloadAutoLimitButtonToggled);
    connect(_downloadValueLimitButton, &CustomRadioButton::toggled, this, &BandwidthDialog::onDownloadValueLimitButtonToggled);
    connect(_downloadValueLimitLineEdit, &QLineEdit::textEdited, this, &BandwidthDialog::onDownloadValueLimitTextEdited);
    connect(_uploadNoLimitButton, &CustomRadioButton::toggled, this, &BandwidthDialog::onUploadNoLimitButtonToggled);
    connect(_uploadAutoLimitButton, &CustomRadioButton::toggled, this, &BandwidthDialog::onUploadAutoLimitButtonToggled);
    connect(_uploadValueLimitButton, &CustomRadioButton::toggled, this, &BandwidthDialog::onUploadValueLimitButtonToggled);
    connect(_uploadValueLimitLineEdit, &QLineEdit::textEdited, this, &BandwidthDialog::onUploadValueLimitTextEdited);
}

void BandwidthDialog::updateUI()
{
    if (_useDownloadLimit == LimitType::NoLimit) {
        if (!_downloadNoLimitButton->isChecked()) {
            _downloadNoLimitButton->setChecked(true);
        }
    }
    else if (_useDownloadLimit == LimitType::Limit) {
        if (!_downloadValueLimitButton->isChecked()) {
            _downloadValueLimitButton->setChecked(true);
        }
    }
    else if (_useDownloadLimit == LimitType::AutoLimit) {
        if (!_downloadAutoLimitButton->isChecked()) {
            _downloadAutoLimitButton->setChecked(true);
        }
    }
    else {
        qCDebug(lcBandwidthDialog) << "Invalid download limit type: " << _useDownloadLimit;
        Q_ASSERT(false);
    }

    bool downloadLimit = _downloadValueLimitButton->isChecked();
    _downloadValueLimitLineEdit->setText(downloadLimit ? QString::number(_downloadLimit) : QString());
    _downloadValueLimitLineEdit->setEnabled(downloadLimit);

    if (_useUploadLimit == LimitType::NoLimit) {
        if (!_uploadNoLimitButton->isChecked()) {
            _uploadNoLimitButton->setChecked(true);
        }
    }
    else if (_useUploadLimit == LimitType::Limit) {
        if (!_uploadValueLimitButton->isChecked()) {
            _uploadValueLimitButton->setChecked(true);
        }
    }
    else if (_useUploadLimit == LimitType::AutoLimit) {
        if (!_uploadAutoLimitButton->isChecked()) {
            _uploadAutoLimitButton->setChecked(true);
        }
    }
    else {
        qCDebug(lcBandwidthDialog) << "Invalid upload limit type: " << _useUploadLimit;
        Q_ASSERT(false);
    }

    bool uploadLimit = _uploadValueLimitButton->isChecked();
    _uploadValueLimitLineEdit->setText(uploadLimit ? QString::number(_uploadLimit) : QString());
    _uploadValueLimitLineEdit->setEnabled(uploadLimit);
}

void BandwidthDialog::setNeedToSave(bool value)
{
    _needToSave = value;
    _saveButton->setEnabled(value);
}

void BandwidthDialog::onExit()
{
    if (_needToSave) {
        CustomMessageBox *msgBox = new CustomMessageBox(
                    QMessageBox::Question,
                    tr("Do you want to save your modifications?"),
                    QMessageBox::Yes | QMessageBox::No, this);
        msgBox->setDefaultButton(QMessageBox::Yes);
        int ret = msgBox->exec();
        if (ret != QDialog::Rejected) {
            if (ret == QMessageBox::Yes) {
                onSaveButtonTriggered();
            }
            else {
                reject();
            }
        }
    }
    else {
        reject();
    }
}

void BandwidthDialog::onSaveButtonTriggered(bool checked)
{
    Q_UNUSED(checked)

    OCC::ConfigFile cfg;
    cfg.setUseDownloadLimit(_useDownloadLimit);
    cfg.setDownloadLimit(_downloadLimit);
    cfg.setUseUploadLimit(_useUploadLimit);
    cfg.setUploadLimit(_uploadLimit);

    OCC::FolderMan::instance()->setDirtyNetworkLimits();

    accept();
}

void BandwidthDialog::onDownloadNoLimitButtonToggled(bool checked)
{
    if (checked && _useDownloadLimit != LimitType::NoLimit) {
        _useDownloadLimit = LimitType::NoLimit;
        updateUI();
        setNeedToSave(true);
    }
}

void BandwidthDialog::onDownloadAutoLimitButtonToggled(bool checked)
{
    if (checked && _useDownloadLimit != LimitType::AutoLimit) {
        _useDownloadLimit = LimitType::AutoLimit;
        updateUI();
        setNeedToSave(true);
    }
}

void BandwidthDialog::onDownloadValueLimitButtonToggled(bool checked)
{
    if (checked && _useDownloadLimit != LimitType::Limit) {
        _useDownloadLimit = LimitType::Limit;
        updateUI();
        setNeedToSave(true);
    }
}

void BandwidthDialog::onDownloadValueLimitTextEdited(const QString &text)
{
    _downloadLimit = text.toInt();
    setNeedToSave(true);
}

void BandwidthDialog::onUploadNoLimitButtonToggled(bool checked)
{
    if (checked && _useUploadLimit != LimitType::NoLimit) {
        _useUploadLimit = LimitType::NoLimit;
        updateUI();
        setNeedToSave(true);
    }
}

void BandwidthDialog::onUploadAutoLimitButtonToggled(bool checked)
{
    if (checked && _useUploadLimit != LimitType::AutoLimit) {
        _useUploadLimit = LimitType::AutoLimit;
        updateUI();
        setNeedToSave(true);
    }

}

void BandwidthDialog::onUploadValueLimitButtonToggled(bool checked)
{
    if (checked && _useUploadLimit != LimitType::Limit) {
        _useUploadLimit = LimitType::Limit;
        updateUI();
        setNeedToSave(true);
    }
}

void BandwidthDialog::onUploadValueLimitTextEdited(const QString &text)
{
    _uploadLimit = text.toInt();
    setNeedToSave(true);
}

}
