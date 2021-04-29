#include "webflowcredentialsdialog.h"

#include <QVBoxLayout>
#include <QLabel>

#include "theme.h"
#include "wizard/owncloudwizardcommon.h"
#include "wizard/webview.h"

namespace OCC {

static const QSize windowSize(625, 630);

static const int infoHMargin = 40;
static const int infoVMargin = 10;

WebFlowCredentialsDialog::WebFlowCredentialsDialog(Account *account, QWidget *parent)
    : KDC::CustomDialog(parent)
    , _webView(nullptr)
{
    Q_UNUSED(account)

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    setMinimumSize(windowSize);
    setMaximumSize(windowSize);

    QVBoxLayout *mainLayout = this->mainLayout();

    _infoLabel = new QLabel();
    _infoLabel->setObjectName("largeMediumBoldTextLabel");
    _infoLabel->setContentsMargins(infoHMargin, infoVMargin, infoHMargin, infoVMargin);
    _infoLabel->setWordWrap(true);
    mainLayout->addWidget(_infoLabel);

    _webView = new WebView();
    mainLayout->addWidget(_webView);

    connect(_webView, &WebView::urlCatched, this, &WebFlowCredentialsDialog::urlCatched);

    _errorLabel = new QLabel();
    _errorLabel->hide();
    mainLayout->addWidget(_errorLabel);

    WizardCommon::initErrorLabel(_errorLabel);

    connect(this, &CustomDialog::exit, this, &WebFlowCredentialsDialog::onExit);
}

WebFlowCredentialsDialog::~WebFlowCredentialsDialog()
{
    if (_webView) {
        delete _webView;
        _webView = nullptr;
    }
}

void WebFlowCredentialsDialog::closeEvent(QCloseEvent* e) {
    Q_UNUSED(e)

    if (_webView) {
        // Force calling WebView::~WebView() earlier so that _profile and _page are
        // deleted in the correct order.
        delete _webView;
        _webView = nullptr;
    }
}

void WebFlowCredentialsDialog::setUrl(const QUrl &url) {
    if (_webView)
        _webView->setUrl(url);
}

void WebFlowCredentialsDialog::setInfo(const QString &msg) {
    _infoLabel->setText(msg);
}

void WebFlowCredentialsDialog::setError(const QString &error) {
    if (error.isEmpty()) {
        _errorLabel->hide();
    } else {
        _errorLabel->setText(error);
        _errorLabel->show();
    }
}

void WebFlowCredentialsDialog::onExit()
{
    reject();
}

}
