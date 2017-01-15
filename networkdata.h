#ifndef NETWORKDATA_H
#define NETWORKDATA_H

#include <QObject>

class NetworkData : public QObject
{
    Q_OBJECT
public:
    explicit NetworkData(QObject *parent = 0);

signals:

public slots:
};

#endif // NETWORKDATA_H