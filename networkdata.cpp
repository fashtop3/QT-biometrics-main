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

    QSettings cnf("Dynamic Drive Technology", "DDTFPBiometric");
    cnf.beginGroup("config");


    QUrl url(cnf.value("server/api-url").toString());
    QUrlQuery urlQuery(url);

    urlQuery.addQueryItem("api_token", cnf.value("server/token").toString());
    url.setQuery(urlQuery);

    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, /*"application/x-www-form-urlencoded"*/ "application/json");

    //                qDebug() << "Posting Data: " << QJsonDocument(*fpJsonObject).toJson().data();

#ifdef QT_DEBUG
    //NOTE: this point is used for debug purpose
    qDebug() << "Calling api url" << url.toString();
#endif
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
