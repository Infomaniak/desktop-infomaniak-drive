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

#include "fileexclusiondialog.h"
#include "fileexclusionnamedialog.h"
#include "custompushbutton.h"
#include "custommessagebox.h"
#include "configfile.h"
#include "folderman.h"
#include "guiutility.h"

#include <QFile>
#include <QHeaderView>
#include <QLabel>
#include <QStandardItem>
#include <QStringList>
#include <QVector>

namespace KDC {

static const int boxHMargin = 40;
static const int boxHSpacing = 10;
static const int titleBoxVMargin = 14;
static const int descriptionBoxVMargin = 20;
static const int hiddenFilesBoxVMargin = 15;
static const int addFileBoxHMargin = 35;
static const int addFileBoxVMargin = 12;
static const int filesTableHeaderBoxVMargin = 15;
static const int filesTableBoxVMargin = 20;

static const int rowHeight = 38;

static QVector<int> tableColumnWidth = QVector<int>()
        << 255  // tableColumn::Pattern
        << 190  // tableColumn::Deletable
        << 35;  // tableColumn::Action

static const int viewIconPathRole = Qt::UserRole;
static const int readOnlyRole = Qt::UserRole + 1;

static const char patternProperty[] = "pattern";

static const char noWarningIndicator = ']';
static const char deletedIndicator = '[';

Q_LOGGING_CATEGORY(lcFileExclusionDialog, "gui.fileexclusiondialog", QtInfoMsg)

FileExclusionDialog::FileExclusionDialog(QWidget *parent)
    : CustomDialog(true, parent)
    , _hiddenFilesCheckBox(nullptr)
    , _filesTableModel(nullptr)
    , _filesTableView(nullptr)
    , _saveButton(nullptr)
    , _actionIconColor(QColor())
    , _actionIconSize(QSize())
    , _needToSave(false)
    , _readOnlyPatternMap(std::map<QString, PatternInfo>())
    , _defaultPatternMap(std::map<QString, PatternInfo>())
    , _userPatternMap(std::map<QString, PatternInfo>())
{
    initUI();
    updateUI();
}

void FileExclusionDialog::initUI()
{
    QVBoxLayout *mainLayout = this->mainLayout();

    // Title
    QLabel *titleLabel = new QLabel(this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    titleLabel->setText(tr("Excluded files"));
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(titleBoxVMargin);

    // Description
    QLabel *descriptionLabel = new QLabel(this);
    descriptionLabel->setObjectName("descriptionLabel");
    descriptionLabel->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    descriptionLabel->setText(tr("Add files or folders that will not be synchronized on your computer."));
    descriptionLabel->setWordWrap(true);
    mainLayout->addWidget(descriptionLabel);
    mainLayout->addSpacing(descriptionBoxVMargin);

    // Synchronize hidden files
    QHBoxLayout *hiddenFilesHBox = new QHBoxLayout();
    hiddenFilesHBox->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    mainLayout->addLayout(hiddenFilesHBox);
    mainLayout->addSpacing(hiddenFilesBoxVMargin);

    _hiddenFilesCheckBox = new CustomCheckBox(this);
    _hiddenFilesCheckBox->setObjectName("hiddenFilesCheckBox");
    _hiddenFilesCheckBox->setText(tr("Synchronize hidden files"));
    hiddenFilesHBox->addWidget(_hiddenFilesCheckBox);

    // Add file button
    QHBoxLayout *addFileHBox = new QHBoxLayout();
    addFileHBox->setContentsMargins(addFileBoxHMargin, 0, addFileBoxHMargin, 0);
    mainLayout->addLayout(addFileHBox);
    mainLayout->addSpacing(addFileBoxVMargin);

    CustomPushButton *addFileButton = new CustomPushButton(":/client/resources/icons/actions/add.svg", tr("Add"), this);
    addFileButton->setObjectName("addFileButton");
    addFileHBox->addWidget(addFileButton);
    addFileHBox->addStretch();

    // Files table header
    QHBoxLayout *filesTableHeaderHBox = new QHBoxLayout();
    filesTableHeaderHBox->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    filesTableHeaderHBox->setSpacing(0);
    mainLayout->addLayout(filesTableHeaderHBox);
    mainLayout->addSpacing(filesTableHeaderBoxVMargin);

    QLabel *header1Label = new QLabel(tr("NAME"), this);
    header1Label->setObjectName("header");
    header1Label->setFixedWidth(tableColumnWidth[tableColumn::Pattern]);
    filesTableHeaderHBox->addWidget(header1Label);

    QLabel *header2Label = new QLabel(tr("NO WARNING"), this);
    header2Label->setObjectName("header");
    header2Label->setFixedWidth(tableColumnWidth[tableColumn::NoWarning]);
    header2Label->setAlignment(Qt::AlignCenter);
    filesTableHeaderHBox->addWidget(header2Label);
    filesTableHeaderHBox->addStretch();

    // Files table view
    QHBoxLayout *filesTableHBox = new QHBoxLayout();
    filesTableHBox->setContentsMargins(boxHMargin, 0, boxHMargin, 0);
    mainLayout->addLayout(filesTableHBox);
    mainLayout->addSpacing(filesTableBoxVMargin);

    _filesTableModel = new QStandardItemModel();
    _filesTableView = new QTableView(this);
    _filesTableView->setModel(_filesTableModel);
    _filesTableView->horizontalHeader()->hide();
    _filesTableView->verticalHeader()->hide();
    _filesTableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    _filesTableView->verticalHeader()->setDefaultSectionSize(rowHeight);
    _filesTableView->setSelectionMode(QAbstractItemView::NoSelection);
    _filesTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    filesTableHBox->addWidget(_filesTableView);
    mainLayout->setStretchFactor(_filesTableView, 1);

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

    connect(_hiddenFilesCheckBox, &CustomCheckBox::clicked, this, &FileExclusionDialog::onHiddenFilesCheckBoxClicked);
    connect(addFileButton, &CustomPushButton::clicked, this, &FileExclusionDialog::onAddFileButtonTriggered);
    connect(_filesTableView, &QTableView::clicked, this, &FileExclusionDialog::onTableViewClicked);
    connect(_saveButton, &QPushButton::clicked, this, &FileExclusionDialog::onSaveButtonTriggered);
    connect(cancelButton, &QPushButton::clicked, this, &FileExclusionDialog::onExit);
    connect(this, &CustomDialog::exit, this, &FileExclusionDialog::onExit);
}

void FileExclusionDialog::updateUI()
{
    _hiddenFilesCheckBox->setChecked(!OCC::FolderMan::instance()->ignoreHiddenFiles());

    // Read only patterns
    _readOnlyPatternMap[".csync_journal.db*"] = PatternInfo();
    _readOnlyPatternMap["._sync_*.db*"] = PatternInfo();
    _readOnlyPatternMap[".sync_*.db*"] = PatternInfo();
    _readOnlyPatternMap[".thumb_*"] = PatternInfo();

    // Default patterns
    OCC::ConfigFile cfgFile;
    readIgnoreFile(cfgFile.excludeFile(OCC::ConfigFile::SystemScope), _defaultPatternMap);

    // User patterns
    readIgnoreFile(cfgFile.excludeFile(OCC::ConfigFile::UserScope), _userPatternMap);

    loadPatternTable();
}

void FileExclusionDialog::readIgnoreFile(const QString &file, std::map<QString, PatternInfo> &patternMap)
{
    QFile ignores(file);
    if (!ignores.open(QIODevice::ReadOnly)) {
        return;
    }

    while (!ignores.atEnd()) {
        QString line = QString::fromUtf8(ignores.readLine());

        // Remove end of line
        if (line.count() > 0 && line[line.count() - 1] == '\n') {
            line.chop(1);
        }
        if (line.count() > 0 && line[line.count() - 1] == '\r') {
            line.chop(1);
        }

        bool noWarning = false;
        bool deleted = false;
        if (line.startsWith(noWarningIndicator)) {
            noWarning = true;
            line = line.mid(1);
        }
        else if (line.startsWith(deletedIndicator)) {
            deleted = true;
            line = line.mid(1);
        }

        patternMap[line] = PatternInfo(noWarning, deleted);
    }
}

void FileExclusionDialog::addPattern(const QString &pattern, const PatternInfo &patternInfo, bool readOnly,
                                     int &row, QString scrollToPattern, int &scrollToRow)
{
    QStandardItem *patternItem = new QStandardItem(pattern);
    QStandardItem *noWarningItem = new QStandardItem();
    QStandardItem *actionItem = new QStandardItem();

    QList<QStandardItem *> itemList;
    itemList.insert(tableColumn::Pattern, patternItem);
    itemList.insert(tableColumn::NoWarning, noWarningItem);
    itemList.insert(tableColumn::Action, actionItem);
    _filesTableModel->appendRow(itemList);

    patternItem->setData(readOnly, readOnlyRole);

    if (readOnly) {
        noWarningItem->setFlags(noWarningItem->flags() ^ Qt::ItemIsEnabled);
        actionItem->setFlags(actionItem->flags() ^ Qt::ItemIsEnabled);
        setActionIcon(actionItem, QString());
    }
    else {
        // Set custom checkbox for No Warning column
        QWidget *noWarningWidget = new QWidget(this);
        QHBoxLayout *noWarningHBox = new QHBoxLayout();
        noWarningWidget->setLayout(noWarningHBox);
        noWarningHBox->setContentsMargins(0, 0, 0, 0);
        noWarningHBox->setAlignment(Qt::AlignCenter);
        CustomCheckBox *noWarningCheckBox = new CustomCheckBox(this);
        noWarningCheckBox->setChecked(patternInfo._noWarning);
        noWarningCheckBox->setAutoFillBackground(true);
        noWarningCheckBox->setObjectName("noWarningCheckBox");
        noWarningCheckBox->setProperty(patternProperty, pattern);
        noWarningHBox->addWidget(noWarningCheckBox);
        int rowNum = _filesTableModel->rowCount() - 1;
        _filesTableView->setIndexWidget(_filesTableModel->index(rowNum, tableColumn::NoWarning), noWarningWidget);

        setActionIcon(actionItem, ":/client/resources/icons/actions/delete.svg");

        connect(noWarningCheckBox, &CustomCheckBox::clicked, this, &FileExclusionDialog::onNoWarningCheckBoxClicked);
    }

    row++;
    if (!scrollToPattern.isEmpty() && pattern == scrollToPattern) {
        scrollToRow = row;
    }
}

void FileExclusionDialog::setActionIconColor(const QColor &color)
{
    _actionIconColor = color;
    setActionIcon();
}

void FileExclusionDialog::setActionIconSize(const QSize &size)
{
    _actionIconSize = size;
    setActionIcon();
}

void FileExclusionDialog::setActionIcon()
{
    if (_actionIconColor != QColor() && _actionIconSize != QSize()) {
        for (int row = 0; row < _filesTableModel->rowCount(); row++) {
            QStandardItem *item = _filesTableModel->item(row, tableColumn::Action);
            QVariant viewIconPathV = item->data(viewIconPathRole);
            if (!viewIconPathV.isNull()) {
                QString viewIconPath = qvariant_cast<QString>(viewIconPathV);
                setActionIcon(item, viewIconPath);
            }
        }
    }
}

void FileExclusionDialog::setActionIcon(QStandardItem *item, const QString &viewIconPath)
{
    if (item) {
        if (item->data(viewIconPathRole).isNull()) {
            item->setData(viewIconPath, viewIconPathRole);
        }
        if (_actionIconColor != QColor() && _actionIconSize != QSize() && !viewIconPath.isEmpty()) {
            item->setData(OCC::Utility::getIconWithColor(viewIconPath, _actionIconColor).pixmap(_actionIconSize),
                          Qt::DecorationRole);
        }
    }
}

void FileExclusionDialog::setNeedToSave(bool value)
{
    _needToSave = value;
    _saveButton->setEnabled(value);
}

void FileExclusionDialog::loadPatternTable(QString scrollToPattern)
{
    int row = -1;
    int scrollToRow = 0;

    _filesTableModel->clear();

    // Read only patterns
    for (auto pattern : _readOnlyPatternMap) {
        addPattern(pattern.first, pattern.second, true,
                   row, scrollToPattern, scrollToRow);
    }

    // Default patterns
    for (auto pattern : _defaultPatternMap) {
        auto patternInfoIt = _userPatternMap.find(pattern.first);
        if (patternInfoIt == _userPatternMap.end()) {
            addPattern(pattern.first, pattern.second, false,
                       row, scrollToPattern, scrollToRow);
        }
        else {
            // Pattern updated or deleted by user
            if (!patternInfoIt->second._deleted) {
                addPattern(patternInfoIt->first, patternInfoIt->second, false,
                           row, scrollToPattern, scrollToRow);
            }
        }
    }

    // User patterns
    for (auto pattern : _userPatternMap) {
        if (_defaultPatternMap.find(pattern.first) == _defaultPatternMap.end()) {
            addPattern(pattern.first, pattern.second, false,
                       row, scrollToPattern, scrollToRow);
        }
    }

    if (scrollToRow) {
        QModelIndex index = _filesTableModel->index(scrollToRow, tableColumn::Pattern);
        _filesTableView->scrollTo(index);
    }

    // Set pattern table columns size
    for (int i = 0; i < tableColumnWidth.count(); i++) {
        _filesTableView->setColumnWidth(i, tableColumnWidth[i]);
    }

    _filesTableView->repaint();
}

void FileExclusionDialog::onExit()
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

void FileExclusionDialog::onHiddenFilesCheckBoxClicked(bool checked)
{
    Q_UNUSED(checked)

    setNeedToSave(true);
}

void FileExclusionDialog::onAddFileButtonTriggered(bool checked)
{
    Q_UNUSED(checked)

    FileExclusionNameDialog *dialog = new FileExclusionNameDialog(this);
    if (dialog->exec() == QDialog::Accepted) {
        QString pattern = dialog->pattern();
        auto defaultPatternIt = _defaultPatternMap.find(pattern);
        auto userPatternIt = _userPatternMap.find(pattern);
        if ((defaultPatternIt == _defaultPatternMap.end()
             && userPatternIt == _userPatternMap.end())
                || (defaultPatternIt != _defaultPatternMap.end()
                    && userPatternIt != _userPatternMap.end()
                    && userPatternIt->second._deleted)) {
            // Add or update pattern
            _userPatternMap[pattern] = PatternInfo();

            // Reload table
            loadPatternTable(pattern);

            setNeedToSave(true);
        }
        else {
            CustomMessageBox *msgBox = new CustomMessageBox(
                        QMessageBox::Question,
                        tr("Pattern already existing, not added!"),
                        QMessageBox::Yes, this);
            msgBox->setDefaultButton(QMessageBox::Yes);
            msgBox->exec();
        }
    }
}

void FileExclusionDialog::onTableViewClicked(const QModelIndex &index)
{
    if (index.isValid()) {
        if (index.column() == tableColumn::Action) {
            QStandardItem *item = _filesTableModel->item(index.row(), tableColumn::Action);
            if (item /*&& item->flags() & Qt::ItemIsEnabled*/) {
                // Delete pattern
                CustomMessageBox *msgBox = new CustomMessageBox(
                            QMessageBox::Question,
                            tr("Do you really want to delete?"),
                            QMessageBox::Yes | QMessageBox::No, this);
                msgBox->setDefaultButton(QMessageBox::No);
                int ret = msgBox->exec();
                if (ret != QDialog::Rejected) {
                    if (ret == QMessageBox::Yes) {
                        QString pattern = _filesTableModel->index(index.row(), tableColumn::Pattern)
                                .data(Qt::DisplayRole).toString();

                        auto patternInfoIt = _defaultPatternMap.find(pattern);
                        if (patternInfoIt == _defaultPatternMap.end()) {
                            _userPatternMap.erase(pattern);
                        }
                        else {
                            _userPatternMap[pattern] = PatternInfo(false, true);
                        }

                        // Reload table
                        loadPatternTable(pattern);

                        setNeedToSave(true);
                    }
                }
            }
        }
    }
}

void FileExclusionDialog::onNoWarningCheckBoxClicked(bool checked)
{
    CustomCheckBox *noWarningCheckBox = qobject_cast<CustomCheckBox *>(sender());
    if (noWarningCheckBox) {
        QString pattern = noWarningCheckBox->property(patternProperty).toString();

        auto patternInfoIt = _defaultPatternMap.find(pattern);
        if (patternInfoIt == _defaultPatternMap.end()) {
            // User pattern
            patternInfoIt = _userPatternMap.find(pattern);
            if (patternInfoIt == _userPatternMap.end()) {
                qCDebug(lcFileExclusionDialog()) << "Pattern not found: " << pattern;
                return;
            }
            else {
                patternInfoIt->second._noWarning = checked;
            }
        }
        else {
            // Default pattern
            patternInfoIt = _userPatternMap.find(pattern);
            if (patternInfoIt == _userPatternMap.end()) {
                // Create user pattern
                _userPatternMap[pattern] = PatternInfo(checked, false);
            }
            else {
                // Delete user pattern
                _userPatternMap.erase(pattern);
            }
        }

        setNeedToSave(true);
    }
    else {
        qCDebug(lcFileExclusionDialog()) << "Null pointer!";
    }
}

void FileExclusionDialog::onSaveButtonTriggered(bool checked)
{
    Q_UNUSED(checked)

    OCC::ConfigFile cfgFile;
    QString ignoreFilePath = cfgFile.excludeFile(OCC::ConfigFile::UserScope);
    QFile ignoreFile(ignoreFilePath);
    if (ignoreFile.open(QIODevice::WriteOnly)) {
        for (auto pattern : _userPatternMap) {
            QByteArray line;
            if (pattern.second._deleted) {
                line += deletedIndicator;
            }
            else if (pattern.second._noWarning) {
                line += noWarningIndicator;
            }
            line += pattern.first.toUtf8() + '\n';

            ignoreFile.write(line);
        }
    }
    else {
        qCWarning(lcFileExclusionDialog()) << "Cannot save file exclusions in " << ignoreFilePath;
        CustomMessageBox *msgBox = new CustomMessageBox(
                    QMessageBox::Warning,
                    tr("Cannot save changes!"),
                    QMessageBox::Ok, this);
        msgBox->exec();
    }
    ignoreFile.close();

    OCC::FolderMan *folderMan = OCC::FolderMan::instance();

    /*
     * Handle the hidden file checkbox
     * The ignoreHiddenFiles flag is a folder specific setting, but for now, it is
     * handled globally. Save it to every folder that is defined.
     */
    folderMan->setIgnoreHiddenFiles(!_hiddenFilesCheckBox->isChecked());

    /*
     * We need to force a remote discovery after a change of the ignore list.
     * Otherwise we would not download the files/directories that are no longer
     * ignored (because the remote etag did not change)
     */
    foreach (OCC::Folder *folder, folderMan->map()) {
        folder->journalDb()->forceRemoteDiscoveryNextSync();
        folder->slotNextSyncFullLocalDiscovery();
        folderMan->scheduleFolder(folder);
    }

    accept();
}

}
