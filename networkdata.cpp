#include "networkdata.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QApplication>
#include <QJsonDocument>
#include <QDebug>

NetworkData::NetworkData(QObject *parent) : QObject(parent)
{

    request.setUrl(QUrl("http://localhost:8000/api/data?api_token=HS4uAfbdFojM46vilOoGgEAJdnsy3u2LXWSJUbVfFf7BbwpXL9A8qK2ChAKq"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, /*"application/x-www-form-urlencoded"*/ "application/json");

    //                qDebug() << "Posting Data: " << QJsonDocument(*fpJsonObject).toJson().data();
}


QNetworkReply* NetworkData::postData(QJsonObject &fpJsonObject)
{
    QNetworkReply *reply = man.post(request, QJsonDocument(fpJsonObject).toJson());

    while(!reply->isFinished())
    {
        qApp->processEvents();
    }

    return reply;
}

QNetworkReply* NetworkData::getData()
{
    QNetworkReply *reply = man.get(request);

    while(!reply->isFinished())
    {
        qApp->processEvents();
    }

    return reply;
}
