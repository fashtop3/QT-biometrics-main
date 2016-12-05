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

    QSettings settings("Dynamic Drive Technology", "DDTFPBiometric");

    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setWindowTitle("Fingerpint Capturing");
    setWindowIcon(QIcon(":/images/icon.jpg"));
}

EVDialog::~EVDialog()
{
    /* unlock the capture.xml file to allow new req */
    xml.unlock();

    delete [] m_RegTemplate.pbData;
    m_RegTemplate.cbData = 0;
    m_RegTemplate.pbData = NULL;

    xml.unlock(); //repeat

    workerThread.quit();
    workerThread.wait();

    delete ui;
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
    GitProcessDialog::fetchUpdates(this);
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
        GitProcessDialog::pushUpdates(xml.data().value("name"), xml.data().value("id"), this);
    }
}

void EVDialog::on_pushButtonEnrollment_clicked()
{
//    onFileChanged(); //just for debug

//    onPullFinished(false); return;

    /* Create fingerprint Enrollement dialog */
    FEDialog dialog;
    connect(&dialog, static_cast<void (FEDialog::*)(bool)>(feDialog->templateGenerated), [&dialog](bool gen){
        if(gen){
            QMessageBox::information(&dialog, "Fingerprint Enrollment", "Template Available for test verification", QMessageBox::Ok);
            dialog.close();
        }
    });

    dialog.exec();

    dialog.getRegTemplate(m_RegTemplate);
}

void EVDialog::on_pushButtonVerification_clicked()
{
    /* verify if there is a copy of finger template */
    if (m_RegTemplate.cbData == 0 || m_RegTemplate.pbData == NULL) {
        QMessageBox::critical(this, "Fingerprint Verification",
                              "Before attempting fingerprint verification, you must either select \"Fingerprint Enrollment\" to create a Fingerprint Enrollment Template, or read a previously saved template using \"Read Fingerprint Enrollment Template\".",
                              QMessageBox::Ok|QMessageBox::Escape);
        return;
    }

    FVDialog dialog;
    /* Load a copyt template for verification */
    dialog.loadRegTemplate(m_RegTemplate);
    dialog.exec();
}

void EVDialog::onPullFinished(bool isError)
{
    qRegisterMetaType<DATA_BLOB>("DATA_BLOB");
    qDebug() << "Calling pull finished" << isError;

    /* get values from QSettings */
    QString winTitle = "FPT-[" + settings.value("studName").toString() + "]-[" + settings.value("studReqID").toString() + "]";

    /* Create finger pting verification dialog */
    feDialog = new FEDialog(this);
    feDialog->setWindowTitle(winTitle);
    feDialog->setModal(true);

    connect(feDialog, static_cast<void (FEDialog::*)(bool)>(feDialog->templateGenerated), [this](bool gen){
        feDialog->nextButtonPtr()->setEnabled(gen);
    });

    connect(feDialog->nextButtonPtr(), &feDialog->nextButtonPtr()->clicked, [this](){
        feDialog->progressBarPtr()->reset();
        /* Close the Fingerprint Matching and Acuisition context to allow reinitialion in the FVDialod ctor */
        feDialog->closeInit();
        /* get copy of raw data sent from the biometric device */
        feDialog->getRawRegTemplate(raw_RegTemplate);

        qDebug() << "From the main thread: " << QThread::currentThreadId();
        fpWorker = new FPDataV(static_cast<QObject*>(feDialog));
//        fpWorker.onStartVerification(raw_RegTemplate);


        fpWorker->moveToThread(&workerThread);

        connect(this, SIGNAL(startVerification(DATA_BLOB)), fpWorker, SLOT(onStartVerification(DATA_BLOB)));
        connect(fpWorker, static_cast<void (FPDataV::*)(int run, int total)>(&fpWorker->progress),
                [this](int run, int total){
            int load = (run/total)*100;
            feDialog->progressBarPtr()->setVisible(1);
            feDialog->progressBarPtr()->setValue(load);
            qDebug() << "run: " << load << "total: " << total;
        });
        connect(fpWorker, static_cast<void (FPDataV::*)(bool isMatched)>(&fpWorker->done),
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
                //copy the file from temp ../ with the req ID
                QString newName = getFPTFilePath("_" + xml.data().value("id") + "_.fpt");
                qDebug() << getFPTTempFilePath("temp.fpt");
                QFile::rename(getFPTTempFilePath("temp.fpt"), newName);

                /* Push all captured fingerprint templates to the remote server */
                GitProcessDialog::pushUpdates(xml.data().value("name"), xml.data().value("id"), this);
            }

//            fpWorker->closeInit();
            delete fpWorker;
            workerThread.exit(0);
            workerThread.wait();
        });

        workerThread.start();
        emit startVerification(raw_RegTemplate);
        feDialog->progressBarPtr()->setVisible(1);
        feDialog->nextButtonPtr()->setEnabled(0);
    });

    if(feDialog->exec())
    {}




//        connect(&worker, static_cast<void (VerifyWorker::*)(bool)> (&VerifyWorker::verifyComplete),
//                this, static_cast<void (EVDialog::*)(bool)> (&EVDialog::onVerifyComplete) );
//        connect(&worker, SIGNAL(verifyComplete(bool)), progDialog, SLOT(close()));






        /* pass the raw template to verify with all existing saved .fpt files*/
//        FPDataV fpdatav(raw_RegTemplate);
//        if(fvdialog.verifyAll())
//        {
//            xml.setStatusCode("401");
//            xml.setStatus("Fingerprint already exists!!!!");
//            xml.setCaptured("0");
//            xml.writeXML();
//            QMessageBox::critical(this, "Fingerprint verification", "Process terminated!!! Fingerprint match found!.", QMessageBox::Close);
//        }
//        else{
//            //copy the file from temp ../ with the req ID
//            QString newName = getFPTFilePath("_" + xml.data().value("id") + "_.fpt");
//            qDebug() << getFPTTempFilePath("temp.fpt");
//            QFile::rename(getFPTTempFilePath("temp.fpt"), newName);

//            /* Push all captured fingerprint templates to the remote server */
//            GitProcessDialog::pushUpdates(xml.data().value("name"), xml.data().value("id"), this);
//        }
//    }
//    else{ //when exec return false
        /* get the template raedy for OnVerification button clicked */
        feDialog->getRegTemplate(m_RegTemplate);
//    }

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

void EVDialog::on_pushButtonClearError_clicked()
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

    QMessageBox::information(this, "Clear Error", "Lock files has been released", QMessageBox::Ok);
}
