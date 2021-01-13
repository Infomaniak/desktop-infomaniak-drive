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

#pragma once

#include "customdialog.h"
#include "customradiobutton.h"

#include <QLineEdit>
#include <QPushButton>

namespace KDC {

class BandwidthDialog : public CustomDialog
{
    Q_OBJECT

public:
    explicit BandwidthDialog(QWidget *parent = nullptr);

private:
    enum LimitType {
        AutoLimit = -1,
        NoLimit,
        Limit
    };

    int _useDownloadLimit;
    int _downloadLimit;
    int _useUploadLimit;
    int _uploadLimit;
    CustomRadioButton *_downloadNoLimitButton;
    CustomRadioButton *_downloadAutoLimitButton;
    CustomRadioButton *_downloadValueLimitButton;
    QLineEdit *_downloadValueLimitLineEdit;
    CustomRadioButton *_uploadNoLimitButton;
    CustomRadioButton *_uploadAutoLimitButton;
    CustomRadioButton *_uploadValueLimitButton;
    QLineEdit *_uploadValueLimitLineEdit;
    QPushButton *_saveButton;
    bool _needToSave;

    void initUI();
    void updateUI();
    void setNeedToSave(bool value);

private slots:
    void onExit();
    void onSaveButtonTriggered(bool checked = false);
    void onDownloadNoLimitButtonToggled(bool checked);
    void onDownloadAutoLimitButtonToggled(bool checked);
    void onDownloadValueLimitButtonToggled(bool checked);
    void onDownloadValueLimitTextEdited(const QString &text);
    void onUploadNoLimitButtonToggled(bool checked);
    void onUploadAutoLimitButtonToggled(bool checked);
    void onUploadValueLimitButtonToggled(bool checked);
    void onUploadValueLimitTextEdited(const QString &text);
};

}

