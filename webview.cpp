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
    setUrl(QUrl("qrc:/index.html"/*"http://localhost:8000/"*/));
}

WebView::~WebView(){}

void WebView::onDoneCapturing(const QString cid, int statusCode, const QString statusText, QString &data)
{
     QString command = QString("doneCapturing('%1', '%2', '%3', '%4');")
             .arg(cid)
             .arg(QString::number(statusCode))
             .arg(statusText)
             .arg(data);

    page()->runJavaScript(command) ;
    QMessageBox::information(this, "Fingerprint enrollment", "Fingerprint capture process completed!!!", QMessageBox::Ok);

//#ifdef QT_DEBUG
    //check response data with debug mode
//    setHtml(data);
//#endif
}

void WebView::jsStartCapturing(const QString cid)
{
    emit initCapturing(cid);
}
