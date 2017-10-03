#include "fedialog.h"
#include "webview.h"
#include <QJsonDocument>
#include <QMessageBox>
#include <QSettings>
#include "webenginepage.h"
#include <QWebEngineProfile>

WebView::WebView(QWidget *parent) :
    QWebEngineView(parent)
{/*
    QString bg = "<img src=':/images/emis.png' width='100%' height='100%' />";
    setHtml(bg);*/

    QSettings cnf("Dynamic Drive Technology", "DDTFPBiometric");
    cnf.beginGroup("config");

    WebEnginePage *page = new WebEnginePage(this);
//    page->profile()->setHttpCacheType(QWebEngineProfile::NoCache);

    page->profile()->clearHttpCache();
    page->load(QUrl(cnf.value("server/main-url").toString()/*"qrc:/index.html"*//*"http://localhost:8000/"*/));
    this->setPage(page);

    // Set up the communications channel
    this->page()->setWebChannel(&channel) ;
    channel.registerObject("widget", this) ;

    // Set the page content
//    setUrl(QUrl(cnf.value("server/main-url").toString()/*"qrc:/index.html"*//*"http://localhost:8000/"*/));

}

WebView::~WebView(){}

void WebView::onDoneCapturing(int statusCode, const QString statusText, QJsonObject &data)
{
    QJsonDocument doc(data);
    QString strJson(doc.toJson(QJsonDocument::Compact));
     QString command = QString("doneCapturing('%1', '%2', '%3');")
             .arg(QString::number(statusCode))
             .arg(statusText)
             .arg(strJson);

    page()->runJavaScript(command) ;

    //NOTE: Done capturing run javascript code

#ifdef QT_DEBUG
    qDebug() << "Run Javascript: " << command;
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
