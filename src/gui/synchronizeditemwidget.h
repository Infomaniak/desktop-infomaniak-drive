#pragma once

#include "synchronizeditem.h"
#include "customtoolbutton.h"

#include <QColor>
#include <QIcon>
#include <QLabel>
#include <QPaintEvent>
#include <QSize>
#include <QWidget>

namespace KDC {

class SynchronizedItemWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QSize file_icon_size READ fileIconSize WRITE setFileIconSize)
    Q_PROPERTY(QColor background_color READ backgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(QColor background_color_selection READ backgroundColorSelection WRITE setBackgroundColorSelection)

public:
    explicit SynchronizedItemWidget(const SynchronizedItem &item, QWidget *parent = nullptr);

    inline QSize fileIconSize() const { return _fileIconSize; }
    inline void setFileIconSize(const QSize &size) {
        _fileIconSize = size;
        emit fileIconSizeChanged();
    }

    inline QColor backgroundColor() const { return _backgroundColor; }
    inline void setBackgroundColor(const QColor &value) { _backgroundColor = value; }

    inline QColor backgroundColorSelection() const { return _backgroundColorSelection; }
    inline void setBackgroundColorSelection(const QColor &value) { _backgroundColorSelection = value; }

    bool isSelected() const { return _isSelected; };
    void setSelected(bool isSelected);

signals:
    void fileIconSizeChanged();

private:
    SynchronizedItem _item;
    bool _isSelected;
    QSize _fileIconSize;
    QColor _backgroundColor;
    QColor _backgroundColorSelection;
    QLabel *_fileIconLabel;
    CustomToolButton *_folderButton;
    CustomToolButton *_menuButton;

    void paintEvent(QPaintEvent* event) override;

    QIcon getIconFromFileName(const QString &fileName) const;

private slots:
    void onFileIconSizeChanged();
    void onFolderButtonClicked();
    void onMenuButtonClicked();
    void onOpenActionTriggered(bool checked = false);
    void onFavoritesActionTriggered(bool checked = false);
    void onRightAndSharingActionTriggered(bool checked = false);
    void onCopyLinkActionTriggered(bool checked = false);
    void onDisplayOnDriveActionTriggered(bool checked = false);
};

}
