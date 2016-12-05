#include "fvdialog.h"
#include "verifyworker.h"
#include <QThread>
#include <QDebug>


VerifyWorker::VerifyWorker(const DATA_BLOB &dataBlob, QObject *parent) : QObject(parent)
{
    raw_RegTempplate = dataBlob;
}

void VerifyWorker::onStartVerification()
{
    bool isMatch = false;
    FVDialog fvdialog;
    isMatch = fvdialog.verifyAll(raw_RegTempplate);
    emit verifyComplete(isMatch);
    onTimeout();
}

void VerifyWorker::onTimeout()
{
    qDebug() << "from the verify thread" << QThread::currentThreadId();
}
