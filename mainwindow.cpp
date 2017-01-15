#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "dpRCodes.h"
#include "dpDefs.h"
#include "DPDevClt.h"
#include "dpFtrEx.h"
#include "dpMatch.h"

#include <QPushButton>
#include <QMessageBox>
#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QApplication>
#include <QThread>

Q_DECLARE_METATYPE(DATA_BLOB);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    webView = new WebView(this);
    connect(webView, SIGNAL(initCapturing(QString)), this, SLOT(onInitCapturing(QString)));
    connect(this, SIGNAL(doneCapturing(QString,int,QString)), webView, SLOT(onDoneCapturing(QString,int,QString)));
    setCentralWidget(webView);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startEnrollment()
{

    qRegisterMetaType<DATA_BLOB>("DATA_BLOB");

    fpJsonObject = new QJsonObject;

    /* Create finger print verification dialog */
    feDialog = new FEDialog(this);
    feDialog->setWindowTitle("Fingerprint Enrollment");
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
//                xml.setStatusCode("401");
//                xml.setStatus("Fingerprint already exists!!!!");
//                xml.setCaptured("0");
//                xml.writeXML();
                QMessageBox::critical(this, "Fingerprint Verification", "Fingerpint match found!!! unable to proceed!.",
                                      QMessageBox::Close);
            }
            else{

                QNetworkRequest request(QUrl("http://localhost:8000/data"));
                request.setHeader(QNetworkRequest::ContentTypeHeader, /*"application/x-www-form-urlencoded"*/ "application/json");

                /* get the template raedy for OnVerification button clicked */
                fpJsonObject->insert("cid", QJsonValue::fromVariant("878274uin"));
                feDialog->getRegTemplate(fpJsonObject);

                QNetworkAccessManager man;
                //                qDebug() << "Posting Data: " << QJsonDocument(*fpJsonObject).toJson().data();
                QNetworkReply *reply = man.post(request, QJsonDocument(*fpJsonObject).toJson());

                while(!reply->isFinished())
                {
                    qApp->processEvents();
                }

                qDebug() << "Server Response: " << reply->readAll().data();

                doneCapturing("cid", 401, "responseText");
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

void MainWindow::onInitCapturing(const QString cid)
{
    try
   {
       DPFPInit();                             //initialize the module

       ULONG device = 0;
       GUID *ppDevUID;
       if (S_OK == DPFPEnumerateDevices(&device, &ppDevUID))    //check device connected
       {
           if(device == 0){
               throw -1;
           }
       }

       if(FT_OK == FX_init())                  //initialize feature extraction
       {
           if(FT_OK == MC_init())              //intialize sdk matching
           {
               //call exec on evdialog
               startEnrollment();

               MC_terminate();                 // All MC_init  must be matched with MC_terminate to free up the resources
           }
           else {
               QMessageBox::critical(0, "Device Error", "Cannot initialize Matching.",
                                     QMessageBox::Close | QMessageBox::Escape);
           }

           FX_terminate();             // All FX_init  must be matched with FX_terminate to free up the resources
       }
       else{
           QMessageBox::critical(0, "Device Error", "Cannot initialize Feature Extraction.",
                                 QMessageBox::Close | QMessageBox::Escape);
       }

       DPFPTerm();                     // All DPFPInit must be matched with DPFPTerm     to free up the resources
   }
   catch (int e)
   {
       qDebug("An exception occurred. Devive not found. ");
       QMessageBox::critical(0, "Device Error", "An exception occurred. Devive not found. ",
                             QMessageBox::Close | QMessageBox::Escape);
       return;
   }
}
