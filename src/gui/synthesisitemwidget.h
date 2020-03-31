#pragma once

#include "synchronizeditem.h"

#include <QIcon>
#include <QPaintEvent>
#include <QString>
#include <QWidget>

namespace KDC {

class SynthesisItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SynthesisItemWidget(const SynchronizedItem &item, QWidget *parent = nullptr);

    void paintEvent(QPaintEvent* event) override;

private:
    SynchronizedItem _item;

    QIcon getIconFromFileName(const QString &fileName) const;
};

}
