#ifndef NETWORKDATA_H
#define NETWORKDATA_H

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QObject>

class NetworkData : public QObject
{
    Q_OBJECT
public:
    explicit NetworkData(QObject *parent = 0);

    QNetworkReply *postData(QJsonObject &fpJsonObject);

signals:
    void configFileError();

public slots:

    QNetworkReply *getData();
protected:
    QNetworkAccessManager man;
    QNetworkRequest request;
};

#endif // NETWORKDATA_H
