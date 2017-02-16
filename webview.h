#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <QWebEngineView>
#include <QWebChannel>
#include "webenginepage.h"


class WebView : public QWebEngineView
{
    Q_OBJECT

public:
    explicit WebView(QWidget *parent = 0);
    virtual ~WebView();

signals:
    void initCapturing(const QString cid);

public slots:
    void onDoneCapturing(const QString cid, int statusCode, const QString statusText, QString &data = QString());
    void jsStartCapturing(const QString cid);

private:

    QWebChannel channel;

    QWebEngineView logView;
//    WebEnginePage *page;

};

#endif // WEBVIEW_H
