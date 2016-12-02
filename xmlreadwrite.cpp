#include "xmlreadwrite.h"

#include <QFile>
#include <QDebug>

XmlReadWrite::XmlReadWrite(QObject *parent) : QObject(parent)
{

}

void XmlReadWrite::setStatus(const QString &status)
{
    dumpData.insert("status", status);
}

void XmlReadWrite::setStatusCode(const QString &code)
{
    dumpData.insert("statusCode", code);
}

void XmlReadWrite::setCaptured(const QString &captured)
{
    dumpData.insert("captured", captured);
}

const QHash<QString, QString> XmlReadWrite::data()
{
    return dumpData;
}

bool XmlReadWrite::writeXML()
{
    QFile file("C:\\Apache24\\cgi-bin\\build-web-dump\\debug\\response.xml");

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << file.errorString();
        return false;
    }

    QByteArray byteArray;
    QXmlStreamWriter stream(&byteArray);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();

         //users
        stream.writeStartElement("users");
            //user
            stream.writeStartElement("user");
            stream.writeAttribute("id", dumpData.value("id"));
            stream.writeAttribute("captured", dumpData.value("captured"));
            stream.writeAttribute("statusCode", dumpData.value("statusCode"));
            stream.writeTextElement("name", dumpData.value("name"));
            stream.writeTextElement("status", dumpData.value("status"));
            stream.writeEndElement();
            // end: user
        stream.writeEndElement();
        //end: users

    stream.writeEndDocument();

    file.write(byteArray);
    file.close();
}

bool XmlReadWrite::readXML()
{
    QFile file("C:\\Apache24\\cgi-bin\\build-web-dump\\debug\\capture.xml");
    /* test to see if the file is readable and that it contains text */
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << file.errorString();
        return false;
    }


    /* QXmlStreamReader takes any QIODevice. */
    QXmlStreamReader xml(&file);
    /* We'll parse the XML until we reach end of it.*/
    while(!xml.atEnd() && !xml.hasError())
    {
        /* Read next element.*/
        QXmlStreamReader::TokenType token = xml.readNext();
        /* If token is just StartDocument, we'll go to next.*/
        if(token == QXmlStreamReader::StartDocument)
            continue;

        /* If token is StartElement, we'll see if we can read it.*/
        if(token == QXmlStreamReader::StartElement) {
            if(xml.name() == "user") {
                /* get the attributes of the xml tag */
                dumpData.insert("id", xml.attributes().value("id").toString());
                dumpData.insert("captured", xml.attributes().value("captured").toString());
                dumpData.insert("statusCode", xml.attributes().value("statusCode").toString());
                /* read next to jump the characters */
                xml.readNext();
                continue;
            }

            if(xml.name() == "name")
            {
                /* we don't need the element attr so shift the pointer to characters */
                xml.readNext();
                dumpData.insert("name", xml.text().toString());
                continue;
            }

            if(xml.name() == "status")
            {
                /* we don't need the element attr so shift the pointer to characters */
                xml.readNext();
                dumpData.insert("status", xml.text().toString());
            }

        }

    }


    return true;
}
