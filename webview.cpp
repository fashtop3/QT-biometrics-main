#include "fedialog.h"
#include "webview.h"
#include <QMessageBox>
#include <QSettings>

WebView::WebView(QWidget *parent) :
    QWebEngineView(parent)
{
    // Set up the communications channel
    this->page()->setWebChannel(&channel) ;
    channel.registerObject("widget", this) ;

    QSettings cnf("Dynamic Drive Technology", "DDTFPBiometric");
    cnf.beginGroup("config");

    // Set the page content
    setUrl(QUrl(cnf.value("server/main-url").toString()/*"qrc:/index.html"*//*"http://localhost:8000/"*/));
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

#ifdef QT_DEBUG
    QMessageBox::information(this, "Fingerprint enrollment", "Fingerprint capture process completed!!!", QMessageBox::Ok);
#endif

//NOTE: this point is used for debug purpose
#ifdef QT_DEBUG
//    check response data with debug mode
//    setHtml(data);
    logView.setHtml(data);
    logView.show();
#endif
}

void WebView::jsStartCapturing(const QString cid)
{
    //NOTE: display user id when button clicked
    qDebug() << "User Id: " << cid;
    emit initCapturing(cid);
}
