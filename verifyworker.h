#ifndef VERIFYWORKER_H
#define VERIFYWORKER_H

#include <QObject>
#include "DPDevClt.h"

class VerifyWorker : public QObject
{
    Q_OBJECT
public:
    explicit VerifyWorker(const DATA_BLOB &dataBlob, QObject *parent = 0);


public slots:
    void onTimeout();
    void onStartVerification();

signals:
    void verifyComplete(bool isMatch);

protected:
    DATA_BLOB raw_RegTempplate;

};

#endif // VERIFYWORKER_H
