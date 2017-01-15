#include "logindialog.h"
#include "ui_logindialog.h"

#include <QNetworkCookie>
#include <QNetworkReply>
#include <QDebug>
#include <QUrlQuery>

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::on_signinPushButton_clicked()
{
    ui->signinPushButton->setEnabled(false);

    QUrl url("http://localhost:8000/api/authenticate");
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("email", "app1");
    urlQuery.addQueryItem("password", "ericson");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, /*"application/x-www-form-urlencoded"*/ "application/json");

    QNetworkAccessManager man;

    QNetworkReply *reply = man.get(request);

    while(!reply->isFinished())
    {
        qApp->processEvents();
    }

    qDebug("reply finished");

    // somehow give reply a value
    qDebug() << reply->readAll().data();

    QList<QByteArray> headerList = reply->rawHeaderList();
    foreach(QByteArray head, headerList) {
        qDebug() << head << ":" << reply->rawHeader(head);
    }

    QVariant cookieVar = reply->header(QNetworkRequest::CookieHeader);
    if (cookieVar.isValid()) {
        qDebug("is valid");
        QList<QNetworkCookie> cookies = cookieVar.value<QList<QNetworkCookie> >();
        foreach (QNetworkCookie cookie, cookies) {
            // do whatever you want here
            qDebug() << cookie.toRawForm().data();
        }
    }

    done(QDialog::Accepted);
}
