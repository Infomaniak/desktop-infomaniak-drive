#pragma once

#include <QWidget>

class WidgetSettings : public QWidget
{
    Q_OBJECT
public:
    explicit WidgetSettings(QWidget *parent = nullptr);
    virtual void customizeStyle();
};
