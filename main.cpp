#include <QApplication>
#include "mainwindow.h"
#include "runguard.h"

#include <QMessageBox>
#include <evdialog.h>
#include <QDebug>
#include <QCommandLineParser>
#include <QThread>
#include <QSplashScreen>
#include <QDesktopWidget>
#include <QPixmap>

class I : public QThread
{
public:
    static void sleep(unsigned long secs) { QThread::sleep(secs); }
};


/**
 * @brief loadSettings prototype
 * @return
 */
bool loadSettings();

int main(int argc, char *argv[])
{

    RunGuard guard( "JFDBHJ87867SHBJKSD7687EKJ");
        if ( !guard.tryToRun() )
            return 0;

    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("DDTFPBiometric");
    QCoreApplication::setOrganizationName("Dynamic Drive Technology");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Student Biometric capturing application");
    parser.addHelpOption();
    parser.addVersionOption();

//    boolean option with a single name (-l)
//    QCommandLineOption xmlFileChangedOption("l", QCoreApplication::translate("main", "Option for capture.xml file changed event"));
//    parser.addOption(xmlFileChangedOption);

    parser.process(app);

//    bool xmlFileChanged = parser.isSet(xmlFileChangedOption);

#ifndef QT_DEBUG
    QPixmap pixmap(":/images/emis.png");
    QSplashScreen splash(pixmap);
    splash.show();
    I::sleep(2);
    splash.activateWindow();
    I::sleep(1); // splash is shown for 5 seconds
#endif

    MainWindow w;
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    int x = (screenGeometry.width()- w.width()) / 2;
    int y = (screenGeometry.height()-w.height()) / 2;
    LoginDialog login;

#ifndef QT_DEBUG
    splash.finish(&w);
#endif

    qDebug() << QApplication::applicationDirPath();
    if(loadSettings())
        w.show();


    return app.exec();
}


bool loadSettings()
{
    QFile file("./config.xml");
    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(0, "Configuration", "Failed to load configuration file. Pls contact administrator!.",
                              QMessageBox::Abort);
        return false;
    }

    QSettings cnf("Dynamic Drive Technology", "DDTFPBiometric");
    cnf.beginGroup("config");

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
            if(xml.name() == "server") {
                /* get the attributes of the xml tag */
//                cnf.insert("id", xml.attributes().value("id").toString());
                /* read next to jump the characters */
                xml.readNext();
                continue;
            }

            if(xml.name() == "main-url")
            {
                /* we don't need the element attr so shift the pointer to characters */
                xml.readNext();
                cnf.setValue("server/main-url", xml.text().toString());
                continue;
            }

            if(xml.name() == "api-url")
            {
                /* get the attributes of the xml tag */
                cnf.setValue("server/token", xml.attributes().value("token").toString());
                /* we don't need the element attr so shift the pointer to characters */
                xml.readNext();
                cnf.setValue("server/api-url", xml.text().toString());
            }

        }
    }

    cnf.endGroup();

    return true;
}
