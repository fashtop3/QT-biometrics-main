#include "fedialog.h"
#include "webview.h"
#include <QMessageBox>

WebView::WebView(QWidget *parent) :
    QWebEngineView(parent)
{

    // Set up the communications channel
    this->page()->setWebChannel(&channel) ;
    channel.registerObject("widget", this) ;

    // Set the page content
    setUrl(QUrl(/*"qrc:/index.html"*/"http://localhost:8000/"));
}

WebView::~WebView(){}

void WebView::onDoneCapturing(const QString cid, int statusCode, const QString statusText)
{
    QString command = QString("doneCapturing('%1', '%2', '%3');")
            .arg(cid)
            .arg(statusCode)
            .arg(statusText);

    page()->runJavaScript(command) ;
    QMessageBox::information(this, "Fingerprint enrollment", "Fingerprint capture process completed!!!", QMessageBox::Ok);
    close();

}

void WebView::jsStartCapturing(const QString str)
{
    emit initCapturing(str);
}
