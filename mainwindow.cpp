#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "dpRCodes.h"
#include "dpDefs.h"
#include "DPDevClt.h"
#include "dpFtrEx.h"
#include "dpMatch.h"
#include "networkdata.h"

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
    setWindowIcon(QIcon(":/images/logo.png"));
    webView = new WebView(this);
    connect(webView, SIGNAL(initCapturing(QString)), this, SLOT(onInitCapturing(QString)));
    connect(this, SIGNAL(doneCapturing(QString,int,QString,QString&)),
            webView, SLOT(onDoneCapturing(QString,int,QString,QString&)));

    connect(webView, SIGNAL(loadStarted()), this, SIGNAL(loadStarted()));
    connect(webView, SIGNAL(loadProgress(int)), this, SIGNAL(loadProgress(int)));
    connect(webView, SIGNAL(loadFinished(bool)), this, SIGNAL(loadFinished(bool)));
    connect(webView->page(), SIGNAL(featurePermissionRequested(QUrl,QWebEnginePage::Feature)),
            this, SLOT(onFeaturePermissionRequested(QUrl,QWebEnginePage::Feature)));

    setCentralWidget(webView);
    setWindowTitle(tr("SBEMIS 2017.1"));
    statusBar()->hide();
}

MainWindow::~MainWindow()
{
    m_RegTemplate.pbData = {0};
    m_RegTemplate.cbData = 0;
    m_RegTemplate.pbData = NULL;

    workerThread.quit();
    workerThread.wait();

    delete ui, fpJsonObject;
}

void MainWindow::startEnrollment()
{

    qRegisterMetaType<DATA_BLOB>("DATA_BLOB");

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

        feDialog->nextButtonPtr()->setEnabled(0); //disable pusbutton
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

        /*Connect the anonymous function to the signal emmited when varification complets*/
        connect(fpWorker, static_cast<void (FPDataV::*)(bool isMatched)>(/*&fpWorker->*/&FPDataV::done),
                [this](bool isMatched){

            if(isMatched) {
                QMessageBox::critical(this, "Fingerprint Verification", "Fingerpint match found!!! unable to proceed!.",
                                      QMessageBox::Close);
                feDialog->done(QDialog::Rejected);
                emit doneCapturing(fpJsonObject->value("cid").toString(), 400, "Fingerpint match found!!! unable to proceed!");
            }
            else{

                /* get the template raedy for OnVerification button clicked */
                feDialog->getRegTemplate(fpJsonObject);

                /**
                 * @brief reply get server response
                 */
                NetworkData networkData(this);
                QNetworkReply *reply = networkData.postData(*fpJsonObject);
                reply->ignoreSslErrors();


                if(reply->isFinished()){
                    if(reply->error()) {
                        QMessageBox::critical(this, "Network Error", reply->errorString(), QMessageBox::Close);
                    }
                    else {
                        QString statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString();
                        QString statusText = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();

                        QString rData = reply->readAll().data();
                        emit doneCapturing(fpJsonObject->value("cid").toString(), statusCode.toInt(), statusText, rData);
                    }
                }

//                QEventLoop loop;
//                connect(reply, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
//                        [this](QNetworkReply::NetworkError code){

//                });

//                connect(reply, static_cast<void(QNetworkReply::*)()>(&QNetworkReply::finishedn),
//                        [this](){
//                    QMessageBox::critical(this, "Network Error", code.errorString(), QMessageBox::Close);
//                });


//                loop.exec();
                feDialog->close();
            }

            fpWorker->deleteLater(); //delete the worker class to allow on next open
            workerThread.exit(0); //exit the thread to enable transfer of new data to thread
            workerThread.wait();
        });

        /*This is where the whole verification processes starts...*/
        workerThread.start(); //thread started
        emit startVerification(raw_RegTemplate); //this signal prompts verification process
        feDialog->progressBarPtr()->setVisible(1);
    });

    feDialog->exec();

    /* get the template raedy for OnVerification button clicked */
    feDialog->getRegTemplate(fpJsonObject);

    feDialog->deleteLater();//, thread;
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
               fpJsonObject = new QJsonObject;
               fpJsonObject->insert("cid", QJsonValue::fromVariant(cid)); //store  cid sent from browser
               startEnrollment();               //start enrollment dialog

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
    }
}

void MainWindow::onFeaturePermissionRequested(QUrl url, QWebEnginePage::Feature feature)
{
    webView->page()->setFeaturePermission(url, feature, QWebEnginePage::PermissionGrantedByUser);
}
