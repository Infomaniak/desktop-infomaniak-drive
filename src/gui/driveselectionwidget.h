#pragma once

#include <unordered_map>

#include <QFont>
#include <QColor>
#include <QLabel>
#include <QList>
#include <QPushButton>
#include <QSize>
#include <QString>

namespace KDC {

class DriveSelectionWidget : public QPushButton
{
    Q_OBJECT

    Q_PROPERTY(QSize drive_icon_size READ driveIconSize WRITE setDriveIconSize)
    Q_PROPERTY(QSize down_icon_size READ downIconSize WRITE setDownIconSize)

public:
    explicit DriveSelectionWidget(QWidget *parent = nullptr);
    QSize sizeHint() const override;

    void addDrive(int id, const QString &name, const QColor &color);
    void selectDrive(int id);

    inline QSize driveIconSize() const { return _driveIconSize; }
    inline void setDriveIconSize(QSize size) {
        _driveIconSize = size;
        emit driveIconSizeChanged();
    }

    inline QSize downIconSize() const { return _downIconSize; }
    inline void setDownIconSize(QSize size) {
        _downIconSize = size;
        emit downIconSizeChanged();
    }

signals:
    void driveIconSizeChanged();
    void downIconSizeChanged();
    void driveSelected(int id);

private:
    QSize _driveIconSize;
    QSize _downIconSize;
    std::unordered_map<int, std::pair<QString, QColor>> _driveMap;
    int _currentDriveId;
    QLabel *_driveIconLabel;
    QLabel *_driveTextLabel;
    QLabel *_downIconLabel;

    void setDriveIcon(const QColor &color);

private slots:
    void onDriveIconSizeChanged();
    void onDownIconSizeChanged();
};

}
