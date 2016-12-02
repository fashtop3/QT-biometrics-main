#ifndef XMLREADWRITE_H
#define XMLREADWRITE_H

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

    explicit XmlReadWrite(QObject *parent = 0);
    void setStatus(const QString &status);
    void setStatusCode(const QString &code);
    void setCaptured(const QString &captured);
    const QHash<QString, QString> data();

signals:

public slots:
    bool writeXML();
    bool readXML(XmlReadWrite::FTYPE fileType);

private:

    QHash<QString, QString> dumpData;
};

#endif // XMLREADWRITE_H
