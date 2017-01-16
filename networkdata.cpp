#include "networkdata.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QApplication>
#include <QJsonDocument>
#include <QDebug>
#include <QFile>
#include <QXmlStreamReader>
#include <QSettings>
#include <QUrlQuery>

NetworkData::NetworkData(QObject *parent) : QObject(parent)
{

    QSettings cnf("Dynamic Drive Technology");
    QUrl url(cnf.value("conifg/server/api-url").toString());
    QUrlQuery urlQuery(url);
    urlQuery.addQueryItem("api_token", cnf.value("conifg/server/token").toString());

    request.setUrl(url);
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
