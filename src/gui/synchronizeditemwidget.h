#pragma once

#include "synchronizeditem.h"

#include <QIcon>
#include <QLabel>
#include <QPaintEvent>
#include <QString>
#include <QWidget>

namespace KDC {

class SynchronizedItemWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor background_color READ backgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(QColor background_color_selection READ backgroundColorSelection WRITE setBackgroundColorSelection)
    Q_PROPERTY(QColor file_name_color READ fileNameColor WRITE setFileNameColor)
    Q_PROPERTY(QColor file_date_color READ fileDateColor WRITE setFileDateColor)

public:
    explicit SynchronizedItemWidget(const SynchronizedItem &item, bool isSelected, QWidget *parent = nullptr);

    void paintEvent(QPaintEvent* event) override;

    inline QColor backgroundColor() const { return _backgroundColor; }
    inline void setBackgroundColor(const QColor& value) { _backgroundColor = value; }

    inline QColor backgroundColorSelection() const { return _backgroundColorSelection; }
    inline void setBackgroundColorSelection(const QColor& value) { _backgroundColorSelection = value; }

    inline QColor fileNameColor() const { return _fileNameColor; }
    inline void setFileNameColor(const QColor& value) {
        _fileNameColor = value;
        emit fileNameColorChanged();
    }

    inline QColor fileDateColor() const { return _fileDateColor; }
    inline void setFileDateColor(const QColor& value) {
        _fileDateColor = value;
        emit fileDateColorChanged();
    }

signals:
    void fileNameColorChanged();
    void fileDateColorChanged();

private:
    SynchronizedItem _item;
    bool _isSelected;
    QString _fileName;
    QString _fileDate;
    QLabel *_fileNameLabel;
    QLabel *_fileDateLabel;
    QColor _backgroundColor;
    QColor _backgroundColorSelection;
    QColor _fileNameColor;
    QColor _fileDateColor;

    QIcon getIconFromFileName(const QString &fileName) const;

private slots:
    void onFileNameColorChanged();
    void onFileDateColorChanged();
};

}
