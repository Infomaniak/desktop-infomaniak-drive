#ifndef WEBFLOWCREDENTIALSDIALOG_H
#define WEBFLOWCREDENTIALSDIALOG_H

#include "customdialog.h"
#include "accountfwd.h"

#include <QDialog>
#include <QUrl>

class QLabel;
class QVBoxLayout;

namespace OCC {

class WebView;

class WebFlowCredentialsDialog : public KDC::CustomDialog
{
    Q_OBJECT
public:
    WebFlowCredentialsDialog(Account *account, QWidget *parent = nullptr);

    void setUrl(const QUrl &url);
    void setInfo(const QString &msg);
    void setError(const QString &error);

protected:
    void closeEvent(QCloseEvent * e) override;

signals:
    void urlCatched(const QString user, const QString pass, const QString host);

private:
    WebView *_webView;

    QLabel *_errorLabel;
    QLabel *_infoLabel;

private slots:
    void onExit();
};

}

#endif // WEBFLOWCREDENTIALSDIALOG_H
