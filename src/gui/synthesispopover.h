#pragma once

#include <QDialog>
#include <QPoint>
#include <QColor>
#include <QEvent>
#include <QFocusEvent>
#include <QPushButton>
#include <QStandardItemModel>
#include <QToolButton>

namespace KDC {

class SynthesisPopover : public QDialog
{
    Q_OBJECT

    Q_PROPERTY(QColor background_main_color READ backgroundMainColor WRITE setBackgroundMainColor)

public:
    explicit SynthesisPopover(QWidget *parent = nullptr);

    inline QColor backgroundMainColor() const { return _backgroundMainColor; }
    inline void setBackgroundMainColor(const QColor& value) { _backgroundMainColor = value; }

    void setSysTrayIconPosition(const QPoint &sysTrayIconPosition);

private:
    QPoint _sysTrayIconPosition;
    QColor _backgroundMainColor;

    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
    bool event(QEvent *event) override;
    void init();
    void populateSynchronizedList(QStandardItemModel *model);
};

}

