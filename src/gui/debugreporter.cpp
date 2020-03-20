#include "debugreporter.h"

#include <QLoggingCategory>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFileInfo>
#include <QFile>
#include <QVBoxLayout>

namespace OCC {

Q_LOGGING_CATEGORY(lcDebugReporter, "gui.debugreporter", QtInfoMsg)

DebugReporter::DebugReporter(const QUrl& url, QWidget *parent)
    : QProgressDialog(tr("Sending of debugging information"), tr("Cancel"), 0, 100, parent)
    , m_reply(0)
    , m_url(url)
{
    setWindowModality(Qt::WindowModal);

    // Add a layout in order to auto-resize to the content
    QVBoxLayout *layout = new QVBoxLayout;
    foreach (QObject *obj, children()) {
        QWidget *widget = qobject_cast<QWidget *>(obj);
        if (widget)
            layout->addWidget(widget);
    }
    setLayout(layout);

    m_request = new QNetworkRequest(m_url);
}

DebugReporter::~DebugReporter()
{
    delete m_request;
    delete m_reply;
}

void DebugReporter::setReportData(const QByteArray &name, const QByteArray &content)
{
    m_formContents.insert(name, content);
}

void DebugReporter::setReportData(const QByteArray &name, const QByteArray &content, const QByteArray &contentType, const QByteArray &fileName)
{
    setReportData(name, content);

    if (!contentType.isEmpty() && !fileName.isEmpty())
    {
        m_formContentTypes.insert(name, contentType);
        m_formFileNames.insert(name, fileName);
    }
}

void DebugReporter::send()
{
    QByteArray body;
    foreach (const QByteArray& name, m_formContents.keys())
    {
        body += "--thkboundary\r\n";
        body += "Content-Disposition: form-data; name=\"" + name + "\"";

        if (!m_formFileNames.value(name).isEmpty() && !m_formContentTypes.value(name).isEmpty())
        {
            body += "; filename=\"" + m_formFileNames.value(name) + "\"\r\n";
            body += "Content-Type: " + m_formContentTypes.value(name) + "\r\n";
        }
        else
        {
            body += "\r\n";
        }

        body += "\r\n";
        body += m_formContents.value(name) + "\r\n";
    }

    body += "--thkboundary\r\n";


    QNetworkAccessManager* nam = new QNetworkAccessManager();
    m_request->setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data; boundary=thkboundary");
    m_reply = nam->post(*m_request, body);

    connect(m_reply, &QNetworkReply::finished, this, &DebugReporter::onDone, Qt::QueuedConnection);
    connect(m_reply, &QNetworkReply::uploadProgress, this, &DebugReporter::onProgress);
}

void DebugReporter::onDone()
{
    QByteArray data = m_reply->readAll();
    QString const response = QString::fromUtf8(data);

    if ((m_reply->error() != QNetworkReply::NoError) || !response.startsWith("DebugID="))
    {
        onFail(m_reply->error(), m_reply->errorString());
    }
    else
    {
        QString debugId = response.split("\n").at(0).split("=").at(1);
        qCDebug(lcDebugReporter) << "Debug report sent:" << debugId;

        emit sent(true);

        reset();
    }
}

void DebugReporter::onProgress(qint64 done, qint64 total)
{
    if (wasCanceled()) {
        m_reply->abort();
    }
    else {
        if (total > 0) {
            setValue(100 * done / total);
        }
    }
}

void DebugReporter::onFail(int error, const QString &errorString)
{
    qCDebug(lcDebugReporter) << "Debug report not sent:" << error << errorString;

    emit done(false);

    reset();
}

} // namespace OCC
