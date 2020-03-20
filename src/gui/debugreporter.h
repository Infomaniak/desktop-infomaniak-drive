#pragma once

#include <QProgressDialog>
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QMap>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>

namespace OCC {

class DebugReporter : public QProgressDialog
{
    Q_OBJECT
public:
    DebugReporter(const QUrl &url, QWidget *parent = nullptr);
    ~DebugReporter();

    void setReportData(const QByteArray &name, const QByteArray &content);
    void setReportData(const QByteArray &name, const QByteArray &content, const QByteArray &contentType, const QByteArray &fileName);

private:
    QNetworkRequest *m_request;
    QNetworkReply *m_reply;
    QUrl m_url;

    QMap<QByteArray, QByteArray> m_formContents;
    QMap<QByteArray, QByteArray> m_formContentTypes;
    QMap<QByteArray, QByteArray> m_formFileNames;

signals:
    void sent(bool retCode);

public slots:
    void send();

private slots:
    void onDone();
    void onProgress(qint64 done, qint64 total);
    void onFail(int error, const QString &errorString);
};

}

