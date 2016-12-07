#ifndef XMLREADWRITE_H
#define XMLREADWRITE_H

#include <QFile>
#include <QHash>
#include <QObject>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class XmlReadWrite : public QObject
{
    Q_OBJECT


public:
    enum FTYPE {
        Capture,
        Response
    };

    explicit XmlReadWrite(QString path = ".", QObject *parent = 0);
    void setStatus(const QString &status);
    void setStatusCode(const QString &code);
    void setCaptured(const QString &captured);
    const QHash<QString, QString> data();

    const QString getFileName() const;
    bool lock();

    bool unlock();
    bool isLocked();
    QByteArray readData();
signals:

public slots:
    bool writeXML();
    bool readXML(XmlReadWrite::FTYPE fileType);
    bool setPath(const QString &path);

private:

    QHash<QString, QString> dumpData;
    QString fPath;
    QFile file;
    QFile lockControl;
};

#endif // XMLREADWRITE_H
