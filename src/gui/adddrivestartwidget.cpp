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

#include "adddrivestartwidget.h"
#include "custommessagebox.h"

#include <QBoxLayout>
#include <QDir>
#include <QLabel>

namespace KDC {

Q_LOGGING_CATEGORY(lcAddDriveStartWidget, "adddrivestartwidget", QtInfoMsg)

AddDriveStartWidget::AddDriveStartWidget(bool autoNext, QWidget *parent)
    : QWidget(parent)
    , _autoNext(autoNext)
    , _accountPtr(nullptr)
    , _serverUrlLineEdit(nullptr)
    , _nextButton(nullptr)
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);

    QHBoxLayout *serverHBoxLayout = new QHBoxLayout();
    serverHBoxLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addLayout(serverHBoxLayout);

    QLabel *serverLabel = new QLabel(this);
    serverLabel->setText(tr("Server :"));
    serverHBoxLayout->addWidget(serverLabel);

    _serverUrlLineEdit = new OCC::PostfixLineEdit(this);
    serverHBoxLayout->addWidget(_serverUrlLineEdit);
    serverHBoxLayout->setStretchFactor(_serverUrlLineEdit, 1);

    mainLayout->addStretch();

    // Add dialog buttons
    QHBoxLayout *buttonsHBox = new QHBoxLayout();
    buttonsHBox->setContentsMargins(0, 0, 0, 0);
    mainLayout->addLayout(buttonsHBox);

    _nextButton = new QPushButton(this);
    _nextButton->setObjectName("defaultbutton");
    _nextButton->setFlat(true);
    _nextButton->setText(tr("NEXT"));
    buttonsHBox->addStretch();
    buttonsHBox->addWidget(_nextButton);

    connect(_serverUrlLineEdit, &QLineEdit::textChanged, this, &AddDriveStartWidget::onUrlChanged);
    connect(_serverUrlLineEdit, &QLineEdit::editingFinished, this, &AddDriveStartWidget::onUrlEditFinished);
    connect(_nextButton, &QPushButton::clicked, this, &AddDriveStartWidget::onNextButtonTriggered);
}

void AddDriveStartWidget::setServerUrl(const QString &url)
{
    _serverUrlLineEdit->setText(url);
    if (_autoNext) {
        onNextButtonTriggered();
    }
}

QString AddDriveStartWidget::serverUrl() const
{
    return _serverUrlLineEdit->text();
}

void AddDriveStartWidget::onUrlChanged(const QString &text)
{
    QString newUrl = text;
    if (text.endsWith("index.php")) {
        newUrl.chop(9);
    }
    if (_accountPtr) {
        QString webDavPath = _accountPtr->davPath();
        if (text.endsWith(webDavPath)) {
            newUrl.chop(webDavPath.length());
        }
        if (webDavPath.endsWith(QDir::separator())) {
            webDavPath.chop(1); // cut off the slash
            if (text.endsWith(webDavPath)) {
                newUrl.chop(webDavPath.length());
            }
        }
    }
    if (newUrl != text) {
        _serverUrlLineEdit->setText(newUrl);
    }

    _nextButton->setEnabled(!newUrl.isEmpty());
}

void AddDriveStartWidget::onUrlEditFinished()
{
    QString url = _serverUrlLineEdit->fullText();
    if (QUrl(url).isRelative() && !url.isEmpty()) {
        // no scheme defined, set one
        url.prepend("https://");
        _serverUrlLineEdit->setFullText(url);
    }
}

void AddDriveStartWidget::onNextButtonTriggered(bool checked)
{
    Q_UNUSED(checked)

    onUrlEditFinished();

    QUrl url(_serverUrlLineEdit->text());
    if (!url.isValid() || url.host().isEmpty()) {
        CustomMessageBox *msgBox = new CustomMessageBox(
                    QMessageBox::Warning,
                    tr("Invalid server URL"),
                    QMessageBox::Ok, this);
        msgBox->exec();
        return;
    }

    emit terminated();
}

}
