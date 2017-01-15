#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <QWebEngineView>
#include <QWebChannel>


class WebView : public QWebEngineView
{
    Q_OBJECT

public:
    explicit WebView(QWidget *parent = 0);
    virtual ~WebView();

signals:
    void initCapturing(const QString cid);

public slots:
    void onDoneCapturing(const QString cid, int statusCode, const QString statusText);
    void jsStartCapturing(const QString str);

private:

    QWebChannel channel;

};

#endif // WEBVIEW_H
