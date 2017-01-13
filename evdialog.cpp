#include "evdialog.h"
#include "fpdatav.h"
#include "gitprocessdialog.h"
#include "ui_evdialog.h"
#include "verifyworker.h"
#include "xmlreadwrite.h"

#include <QFile>
#include <QMessageBox>
#include <QtWin>
#include <QDebug>
#include <QProcess>
#include <QSettings>
#include <QTimer>
#include <QNetworkRequest>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "fptpath.h"

Q_DECLARE_METATYPE(DATA_BLOB);


EVDialog::EVDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EVDialog)
{
    ui->setupUi(this);
    lockFile = 0;
    /* fill the memory with zeros */
    ::ZeroMemory(&m_RegTemplate, sizeof(m_RegTemplate));
    ::ZeroMemory(&raw_RegTemplate, sizeof(raw_RegTemplate));

    /* set flag to alway show on top */
//    setAttribute(Qt::WA_ShowWithoutActivating);

    fpJsonObject = 0;

    QSettings settings("Dynamic Drive Technology", "DDTFPBiometric");

    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setWindowTitle("Fingerpint Capturing");
    setWindowIcon(QIcon(":/images/icon.jpg"));
}

EVDialog::~EVDialog()
{
    /* unlock the capture.xml file to allow new req */
    xml.unlock();
    removeLockFiles(); //force remove *.lock files

    delete [] m_RegTemplate.pbData;
    m_RegTemplate.cbData = 0;
    m_RegTemplate.pbData = NULL;

    xml.unlock(); //repeat

    workerThread.quit();
    workerThread.wait();

    delete ui, fpJsonObject;
}

/**
 * @brief EVDialog::onFileChanged
 * This method is called when capture.xml file changed
 */
void EVDialog::onFileChanged()
{
    QString pathToXml = QCoreApplication::applicationDirPath() + "/Apache24/cgi-bin";
    /* xml open to read from the cgi server-path */
    xml.setPath(pathToXml);
    xml.readXML(XmlReadWrite::Capture);
    xml.lock();

    /* setting global variables for windows title */
    settings.setValue("studName", xml.data().value("name"));
    settings.setValue("studReqID", xml.data().value("id"));

    /* lunch fetc update dialog */
//    GitProcessDialog::fetchUpdates(this);
    //TODO: do json update
}

void EVDialog::onVerifyComplete(bool isMatched)
{
//    progDialog->close();
    if(isMatched)
    {
        xml.setStatusCode("401");
        xml.setStatus("Fingerprint already exists!!!!");
        xml.setCaptured("0");
        xml.writeXML();
        QMessageBox::critical(this, "Fingerprint verification", "Process terminated!!! Fingerprint match found!.", QMessageBox::Close);
    }
    else{
        //copy the file from temp ../ with the req ID
        QString newName = getFPTFilePath("_" + xml.data().value("id") + "_.fpt");
        qDebug() << getFPTTempFilePath("temp.fpt");
        QFile::rename(getFPTTempFilePath("temp.fpt"), newName);

        /* Push all captured fingerprint templates to the remote server */
//        GitProcessDialog::pushUpdates(xml.data().value("name"), xml.data().value("id"), this);
        //TODO: post sample to server
    }
}

void EVDialog::on_pushButtonEnrollment_clicked()
{
//    onFileChanged(); //just for debug

    onPullFinished(false); return;

    /* Create fingerprint Enrollement dialog */
    FEDialog dialog;
    connect(&dialog, static_cast<void (FEDialog::*)(bool)>(/*feDialog->*/&FEDialog::templateGenerated), [&dialog](bool gen){
        if(gen){
            QMessageBox::information(&dialog, "Fingerprint Enrollment", "Template Available for test verification", QMessageBox::Ok);
            dialog.close();
        }
    });

    dialog.exec();

    if(dialog.isTemplateReady()) {
        fpJsonObject = new QJsonObject;
        dialog.getRegTemplate(fpJsonObject);
    }


}

void EVDialog::on_pushButtonVerification_clicked()
{
    /* verify if there is a copy of finger template */
    if (fpJsonObject == 0) {
        QMessageBox::critical(this, "Fingerprint Verification",
                              "Before attempting fingerprint verification, you must either select \"Fingerprint Enrollment\" to create a Fingerprint Enrollment Template, or read a previously saved template using \"Read Fingerprint Enrollment Template\".",
                              QMessageBox::Ok|QMessageBox::Escape);
        return;
    }
//    return;

    FVDialog dialog;
    /* Load a copyt template for verification */
    dialog.loadRegTemplate(fpJsonObject/*m_RegTemplate*/);
    dialog.exec();
    fpJsonObject = 0;
}

void EVDialog::onPullFinished(bool isError)
{
    qRegisterMetaType<DATA_BLOB>("DATA_BLOB");
    qDebug() << "Calling pull finished" << isError;

    fpJsonObject = new QJsonObject;

    /* get values from QSettings */
    QString winTitle = "FPT-[" + settings.value("studName").toString() + "]-[" + settings.value("studReqID").toString() + "]";

    /* Create finger print verification dialog */
    feDialog = new FEDialog(this);
    feDialog->setWindowTitle(winTitle);
    feDialog->setModal(true);

    /*connect fingerprint template generated to enable next button*/
    connect(feDialog, static_cast<void (FEDialog::*)(bool)>(/*feDialog->*/&FEDialog::templateGenerated), [this](bool gen){
        feDialog->nextButtonPtr()->setEnabled(gen);
    });


    /*when next button is clicked*/
    connect(feDialog->nextButtonPtr(), static_cast<void (QPushButton::*)(bool)>(&QPushButton::clicked), [this](){
        /*reset progress bar*/
        feDialog->progressBarPtr()->reset();
        /* Close the Fingerprint Matching and Acuisition context to allow reinitialion in the FVDialod ctor */
        feDialog->closeInit();
        /* get copy of raw data sent from the biometric device */
        feDialog->getRawRegTemplate(raw_RegTemplate);

        qDebug() << "From the main thread: " << QThread::currentThreadId();
        /*Create new data class for matching BLOB data*/
        fpWorker = new FPDataV(static_cast<QObject*>(feDialog));
        /*move to new Thread apart from the gui thread*/
        fpWorker->moveToThread(&workerThread);

        /*connect slot to start full verification*/
        connect(this, SIGNAL(startVerification(DATA_BLOB)), fpWorker, SLOT(onStartVerification(DATA_BLOB)));
        /*Connect/Get verification updates from the data class tp update the progress bar*/
        connect(fpWorker, static_cast<void (FPDataV::*)(int run, int total)>(/*&fpWorker->*/&FPDataV::progress),
                [this](int run, int total){
            int load = (run/total)*100;
            feDialog->progressBarPtr()->setVisible(1);
            feDialog->progressBarPtr()->setValue(load);
        });//

        /*Connect the anonymous function to the signal emmmited when varification complets*/
        connect(fpWorker, static_cast<void (FPDataV::*)(bool isMatched)>(/*&fpWorker->*/&FPDataV::done),
                [this](bool isMatched){

            if(isMatched) {
                xml.setStatusCode("401");
                xml.setStatus("Fingerprint already exists!!!!");
                xml.setCaptured("0");
                xml.writeXML();
                QMessageBox::critical(this, "Fingerprint Verification", "Fingerpint match found!!! unable to proceed!.",
                                      QMessageBox::Close);
            }
            else{

                QNetworkRequest request(QUrl("http://localhost:8000/data"));
                request.setHeader(QNetworkRequest::ContentTypeHeader, /*"application/x-www-form-urlencoded"*/ "application/json");

                /* get the template raedy for OnVerification button clicked */
                fpJsonObject->insert("cid", QJsonValue::fromVariant(xml.data().value("id")));
                feDialog->getRegTemplate(fpJsonObject);

                QNetworkAccessManager man;
//                qDebug() << "Posting Data: " << QJsonDocument(*fpJsonObject).toJson().data();
                QNetworkReply *reply = man.post(request, QJsonDocument(*fpJsonObject).toJson());

                while(!reply->isFinished())
                {
                    qApp->processEvents();
                }

                qDebug() << "Server Response: " << reply->readAll().data();

                onPushFinished(false);
            }

            fpWorker->deleteLater(); //delete the worker class to allow on next open
            workerThread.exit(0); //exit the thread to enable transfer of new data to thread
            workerThread.wait();
        });

        /*This is where the whole verification processes starts...*/
        workerThread.start(); //thread started
        emit startVerification(raw_RegTemplate); //this signal prompts verification process
        feDialog->progressBarPtr()->setVisible(1);
        feDialog->nextButtonPtr()->setEnabled(0); //disable pusbutton

    });

    feDialog->exec();

        /* get the template raedy for OnVerification button clicked */
        feDialog->getRegTemplate(fpJsonObject);

    delete feDialog;//, thread;
}

void EVDialog::onPushFinished(bool isError)
{
    Q_UNUSED(isError)

    xml.setStatusCode("200");
    xml.setStatus("Fingerprint captured!!!");
    xml.setCaptured("1");
    xml.writeXML();
    QMessageBox::information(this, "Fingerprint enrollment", "Fingerprint capture process completed!!!", QMessageBox::Ok);
    QApplication::closeAllWindows();
}

void EVDialog::removeLockFiles()
{
    QDir dir(QCoreApplication::applicationDirPath() + "/Apache24/cgi-bin");
    dir.setFilter(QDir::Files|QDir::NoDotAndDotDot);
    QFileInfoList files = dir.entryInfoList(QStringList() << "*.lock");
    QListIterator<QFileInfo> i(files);
    while(i.hasNext()) {
        QFileInfo fileInfo = i.next(); //get the filename
        QFile lockedFile(fileInfo.absoluteFilePath());
        lockedFile.remove();
    }
}

void EVDialog::on_pushButtonClearError_clicked()
{
    removeLockFiles();

    QMessageBox::information(this, "Clear Error", "Lock files has been released", QMessageBox::Ok);
    QApplication::closeAllWindows();
}
