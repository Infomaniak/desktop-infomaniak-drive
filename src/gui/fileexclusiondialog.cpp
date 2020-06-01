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
#include "configfile.h"
#include "folderman.h"
#include "guiutility.h"

#include <QFile>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QStandardItem>
#include <QStringList>
#include <QVector>

namespace KDC {

static const int boxHMargin = 40;
static const int boxHSpacing = 10;
static const int titleBoxVMargin = 14;
static const int descriptionBoxVMargin = 20;
static const int hiddenFilesBoxVMargin = 15;
static const int addFileBoxHMargin = 30;
static const int addFileBoxVMargin = 12;
static const int filesTableHeaderBoxVMargin = 15;
static const int filesTableBoxVMargin = 20;

static const int rowHeight = 38;

static QVector<int> tableColumnWidth = QVector<int>()
        << 255  // tableColumn::Pattern
        << 190  // tableColumn::Deletable
        << 35;  // tableColumn::Action

static const int viewIconPathRole = Qt::UserRole;
static const int skippedLinesRole = Qt::UserRole + 1;
static const int isGlobalRole = Qt::UserRole + 2;

FileExclusionDialog::FileExclusionDialog(QWidget *parent)
    : CustomDialog(true, parent)
    , _hiddenFilesCheckBox(nullptr)
    , _filesTableModel(nullptr)
    , _filesTableView(nullptr)
    , _saveButton(nullptr)
    , _actionIconColor(QColor())
    , _actionIconSize(QSize())
    , _needToSave(false)
{
    setStyleSheet("QTableView::indicator:checked { image: url(:/client/resources/icons/actions/checkbox-checked.svg); }"
                  "QTableView::indicator:unchecked { image: url(:/client/resources/icons/actions/checkbox-unchecked.svg); }"
                  "QTableView::indicator:checked:disabled { image: url(:/client/resources/icons/actions/checkbox-checked.svg); }"
                  "QTableView::indicator:unchecked:disabled { image: url(:/client/resources/icons/actions/checkbox-unchecked.svg); }");

    initUI();
    updateUI();
}

void FileExclusionDialog::initUI()
{
    QVBoxLayout *mainLayout = this->mainLayout();

    // Title
    QLabel *titleLabel = new QLabel(this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setContentsMargins(boxHMargin, 0, boxHMargin, titleBoxVMargin);
    titleLabel->setText(tr("Excluded files"));
    mainLayout->addWidget(titleLabel);

    // Description
    QLabel *descriptionLabel = new QLabel(this);
    descriptionLabel->setObjectName("descriptionLabel");
    descriptionLabel->setContentsMargins(boxHMargin, 0, boxHMargin, descriptionBoxVMargin);
    descriptionLabel->setText(tr("Add files or folders that will not be synchronized on your computer."));
    mainLayout->addWidget(descriptionLabel);

    // Synchronize hidden files
    QHBoxLayout *hiddenFilesHBox = new QHBoxLayout();
    hiddenFilesHBox->setContentsMargins(boxHMargin, 0, boxHMargin, hiddenFilesBoxVMargin);
    mainLayout->addLayout(hiddenFilesHBox);

    _hiddenFilesCheckBox = new CustomCheckBox(this);
    _hiddenFilesCheckBox->setObjectName("hiddenFilesCheckBox");
    _hiddenFilesCheckBox->setText(tr("Synchronize hidden files"));
    hiddenFilesHBox->addWidget(_hiddenFilesCheckBox);

    // Add file button
    QHBoxLayout *addFileHBox = new QHBoxLayout();
    addFileHBox->setContentsMargins(addFileBoxHMargin, 0, addFileBoxHMargin, addFileBoxVMargin);
    mainLayout->addLayout(addFileHBox);

    CustomPushButton *addFileButton = new CustomPushButton(":/client/resources/icons/actions/add.svg", tr("Add"), this);
    addFileButton->setObjectName("addFileButton");
    addFileHBox->addWidget(addFileButton);
    addFileHBox->addStretch();

    // Files table header
    QHBoxLayout *filesTableHeaderHBox = new QHBoxLayout();
    filesTableHeaderHBox->setContentsMargins(boxHMargin, 0, boxHMargin, filesTableHeaderBoxVMargin);
    filesTableHeaderHBox->setSpacing(0);
    mainLayout->addLayout(filesTableHeaderHBox);

    QLabel *header1Label = new QLabel(tr("NAME"), this);
    header1Label->setObjectName("header");
    header1Label->setFixedWidth(tableColumnWidth[tableColumn::Pattern]);
    filesTableHeaderHBox->addWidget(header1Label);

    QLabel *header2Label = new QLabel(tr("ALLOW DELETION"), this);
    header2Label->setObjectName("header");
    header2Label->setFixedWidth(tableColumnWidth[tableColumn::Deletable]);
    header2Label->setAlignment(Qt::AlignCenter);
    filesTableHeaderHBox->addWidget(header2Label);
    filesTableHeaderHBox->addStretch();

    // Files table view
    QHBoxLayout *filesTableHBox = new QHBoxLayout();
    filesTableHBox->setContentsMargins(boxHMargin, 0, boxHMargin, filesTableBoxVMargin);
    mainLayout->addLayout(filesTableHBox);

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
    _hiddenFilesCheckBox->setChecked(OCC::FolderMan::instance()->ignoreHiddenFiles());

    OCC::ConfigFile cfgFile;
    addPattern(".csync_journal.db*", /*deletable=*/false, /*readonly=*/true, /*global=*/true);
    addPattern("._sync_*.db*", /*deletable=*/false, /*readonly=*/true, /*global=*/true);
    addPattern(".sync_*.db*", /*deletable=*/false, /*readonly=*/true, /*global=*/true);
    readIgnoreFile(cfgFile.excludeFile(OCC::ConfigFile::SystemScope), /*global=*/true);
    readIgnoreFile(cfgFile.excludeFile(OCC::ConfigFile::UserScope), /*global=*/false);
}

void FileExclusionDialog::readIgnoreFile(const QString &file, bool global)
{
    QFile ignores(file);
    if (!ignores.open(QIODevice::ReadOnly)) {
        return;
    }

    QStringList skippedLines;
    bool readonly = global; // global ignores default to read-only

    while (!ignores.atEnd()) {
        QString line = QString::fromUtf8(ignores.readLine());

        // Remove end of line
#ifdef Q_OS_WINDOWS
        line.chop(2);
#else
        line.chop(1);
#endif

        // Collect empty lines and comments, we want to preserve them
        if (line.isEmpty() || line.startsWith("#")) {
            skippedLines.append(line);
            // A directive that prohibits editing in the ui
            if (line == "#!readonly") {
                readonly = true;
            }
            continue;
        }

        bool deletable = false;
        if (line.startsWith(']')) {
            deletable = true;
            line = line.mid(1);
        }

        // Add and reset
        addPattern(line, deletable, readonly, global, skippedLines);
        skippedLines.clear();
        readonly = global;
    }

    // Set columns size
    for (int i = 0; i < tableColumnWidth.count(); i++) {
        _filesTableView->setColumnWidth(i, tableColumnWidth[i]);
    }
}

void FileExclusionDialog::addPattern(const QString &pattern, bool deletable, bool readOnly, bool global, const QStringList &skippedLines)
{
    QStandardItem *patternItem = new QStandardItem(pattern);
    patternItem->setData(skippedLines, skippedLinesRole);
    patternItem->setData(global, isGlobalRole);

    QStandardItem *deletableItem = new QStandardItem();

    QStandardItem *viewItem = new QStandardItem();
    setActionIcon(viewItem, ":/client/resources/icons/actions/delete.svg");

    QList<QStandardItem *> row;
    row.insert(tableColumn::Pattern, patternItem);
    row.insert(tableColumn::Deletable, deletableItem);
    row.insert(tableColumn::Action, viewItem);
    _filesTableModel->appendRow(row);

    if (readOnly) {
        patternItem->setFlags(patternItem->flags() ^ Qt::ItemIsEnabled);
        deletableItem->setFlags(deletableItem->flags() ^ Qt::ItemIsEnabled);
    }

    // Set custom checkbox for Deletable column
    QWidget *deletableWidget = new QWidget(this);
    QHBoxLayout *deletableHBox = new QHBoxLayout();
    deletableWidget->setLayout(deletableHBox);
    deletableHBox->setContentsMargins(0, 0, 0, 0);
    deletableHBox->setAlignment(Qt::AlignCenter);
    CustomCheckBox *deletableCheckBox = new CustomCheckBox(this);
    deletableCheckBox->setChecked(deletable);
    deletableCheckBox->setAutoFillBackground(true);
    deletableCheckBox->setObjectName("deletableCheckBox");
    deletableHBox->addWidget(deletableCheckBox);
    int rowNum = _filesTableModel->rowCount() - 1;
    _filesTableView->setIndexWidget(_filesTableModel->index(rowNum, tableColumn::Deletable),
                                    deletableWidget);
    connect(deletableCheckBox, &CustomCheckBox::clicked, this, &FileExclusionDialog::onDeletableCheckBoxClicked);
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
        if (_actionIconColor != QColor() && _actionIconSize != QSize()) {
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

void FileExclusionDialog::onExit()
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
        addPattern(pattern, /*deletable=*/false, /*readonly=*/false, /*global=*/false);
        _filesTableView->repaint();
        _filesTableView->scrollToBottom();
        setNeedToSave(true);
    }
}

void FileExclusionDialog::onTableViewClicked(const QModelIndex &index)
{
    if (index.isValid()) {
        if (index.column() == tableColumn::Action) {
            QStandardItem *item = _filesTableModel->item(index.row(), tableColumn::Deletable);
            if (item && item->flags() & Qt::ItemIsEnabled) {
                // Delete
                QMessageBox msgBox(QMessageBox::Question, QString(),
                                   tr("Do you really want to delete?"),
                                   QMessageBox::Yes | QMessageBox::No, this);
                msgBox.setWindowModality(Qt::WindowModal);
                msgBox.setDefaultButton(QMessageBox::No);
                if(msgBox.exec() == QMessageBox::Yes) {
                    _filesTableModel->removeRow(index.row());
                    _filesTableView->repaint();
                    setNeedToSave(true);
                }
            }
        }
    }
}

void FileExclusionDialog::onDeletableCheckBoxClicked(bool checked)
{
    Q_UNUSED(checked)

    setNeedToSave(true);
}

void FileExclusionDialog::onSaveButtonTriggered(bool checked)
{
    Q_UNUSED(checked)

    OCC::ConfigFile cfgFile;
    QString ignoreFilePath = cfgFile.excludeFile(OCC::ConfigFile::UserScope);
    QFile ignoreFile(ignoreFilePath);
    if (ignoreFile.open(QIODevice::WriteOnly)) {
        for (int row = 0; row < _filesTableModel->rowCount(); ++row) {
            QStandardItem *patternItem = _filesTableModel->item(row, tableColumn::Pattern);

            QWidget *deletableWidget = qobject_cast<QWidget *>(
                        _filesTableView->indexWidget(_filesTableModel->index(row, tableColumn::Deletable)));
            if (!deletableWidget) {
               ASSERT(false);
            }
            CustomCheckBox *deletableCheckBox = deletableWidget->findChild<CustomCheckBox *>();
            if (!deletableCheckBox) {
               ASSERT(false);
            }

            if (patternItem->data(isGlobalRole).toBool()) {
                continue;
            }

            QStringList skippedLines = patternItem->data(skippedLinesRole).toStringList();
            for (const auto &line : skippedLines) {
                ignoreFile.write(line.toUtf8() + '\n');
            }

            QByteArray prepend;
            if (deletableCheckBox->isChecked()) {
                prepend = "]";
            } else if (patternItem->text().startsWith('#')) {
                prepend = "\\";
            }
            ignoreFile.write(prepend + patternItem->text().toUtf8() + '\n');
        }
    } else {
        QMessageBox msgBox(QMessageBox::Warning, tr("Could not open file"),
                    tr("Cannot write changes to '%1'.").arg(ignoreFilePath),
                    QMessageBox::Ok, this);
        msgBox.setWindowModality(Qt::WindowModal);
        msgBox.exec();
    }
    ignoreFile.close();

    OCC::FolderMan *folderMan = OCC::FolderMan::instance();

    /*
     * Handle the hidden file checkbox
     * The ignoreHiddenFiles flag is a folder specific setting, but for now, it is
     * handled globally. Save it to every folder that is defined.
     */
    folderMan->setIgnoreHiddenFiles(_hiddenFilesCheckBox->isChecked());

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
