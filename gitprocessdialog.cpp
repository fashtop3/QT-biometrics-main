#include "gitprocessdialog.h"
#include "ui_gitprocessdialog.h"
#include "fptpath.h"

#include <QProcess>
#include <QDebug>
#include <QScrollBar>
#include <QColorDialog>
#include <QMessageBox>
#include  <QTimer>
#include <QSettings>

GitProcessDialog::GitProcessDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GitProcessDialog),
    isErrorFound(false),
    isUploadDone(false)
{
    ui->setupUi(this);
//    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
//    setWindowFlags(Qt::WindowStaysOnTopHint);

    process = new QProcess(this); //create new process
    connect(process, &QProcess::readyReadStandardOutput,
            this, &GitProcessDialog::onReadyReadStandardOutput);

    connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
          [=](int exitCode, QProcess::ExitStatus exitStatus){
            qDebug("process finished");

            if(!isErrorFound){
                ui->pushButtonNext->setEnabled(true);
            }

    });

    ui->textEdit->setReadOnly(true);


    /* Change the look of the TextEdit */
    textEditPallete.setColor(QPalette::Base, Qt::black); // set color "Red" for textedit base
    textEditPallete.setColor(QPalette::Text, Qt::green); // set text color which is selected from color pallete
    ui->textEdit->setPalette(textEditPallete); // change textedit palette

    timerActivate = startTimer(2000);
}

GitProcessDialog::~GitProcessDialog()
{
    delete ui;
}

QSettings GitProcessDialog::settings("Dynamic Drive Technology", "DDTFPBiometric"); // make static variable available
QString GitProcessDialog::winTitle = "FPT-[" + settings.value("studName").toString() + "]-[" + settings.value("studReqID").toString() + "]"; // make static variable available

void GitProcessDialog::fetchUpdates(EVDialog *parent)
{
    GitProcessDialog processDialog;
    processDialog.setWindowTitle(winTitle);
    if(parent){ //connect pull signal to the main EVdialog onEvent
        connect(&processDialog, &GitProcessDialog::pullFinished,
                parent, &parent->onPullFinished);
    }
    processDialog.pull();
    processDialog.setModal(true);
    if(!processDialog.exec())
        QApplication::closeAllWindows();
}

void GitProcessDialog::pushUpdates(const QString name, const QString id, EVDialog *parent)
{
    GitProcessDialog processDialog(parent);
    processDialog.setWindowTitle(winTitle);
    if(parent){ //connect push signal to the main EVdialog onEvent
        connect(&processDialog, &GitProcessDialog::pushFinished,
                parent, &parent->onPushFinished);
    }
    processDialog.push(name, id);
    processDialog.setModal(true);
    if(!processDialog.exec())
        QApplication::closeAllWindows();
}

void GitProcessDialog::push(QString arg1, QString arg2)
{
    QString batpath = QString(_FPT_PATH_()) + "/";
    QString push = QString("git_push.bat \"%1\" \"%2\"")
            .arg(arg1)
            .arg(arg2);

    QString commandStr = QString("cmd.exe") + " /c cd " + batpath + " && " + push;

    //set new working directory for the process
    process->setWorkingDirectory(batpath);
    process->start(commandStr);
}

void GitProcessDialog::pull()
{
    QString batpath = QString(_FPT_PATH_()) + "/";
    QString pull = "git_pull.bat";

    QString commandStr = QString("cmd.exe") + " /c cd " + batpath + " && " + pull;

    //set new working directory for the process
    process->setWorkingDirectory(batpath);
    process->start(commandStr);

}

void GitProcessDialog::onReadyReadStandardOutput()
{
    static int seek  = 0;
    if(seek == 0)
        ui->textEdit->clear();

    /* get all standard output */
    QString stdOutput = process->readAllStandardOutput().data();
    /* get all standard error output */
    QString stdError = process->readAllStandardError().data();

    /* check if the standard error output is not empty */
    if(!stdError.isEmpty()) {
        ui->textEdit->setTextColor(QColor(Qt::red));
        ui->textEdit->append(stdError);
        isErrorFound = true;
        /* this point checks in when the file has been pushed to the remote repository*/
        if(stdError.contains("To ssh")) {
            isUploadDone = true;
            /* change the push button text and enable it to mark git push completion */
            ui->pushButtonNext->setText(tr("&Completed"));
            ui->pushButtonNext->setEnabled(true);
        }
    }

     /* check if the standard output is not empty */
    if(!stdOutput.isEmpty()){
        ui->textEdit->setTextColor(QColor(Qt::green));
        ui->textEdit->append(stdOutput);
    }

    /* control the textEdit scroll bar to the bottom */
    QScrollBar *vScroll = ui->textEdit->verticalScrollBar();
    vScroll->setValue(vScroll->maximum());

    ++seek;
}

void GitProcessDialog::on_pushButtonNext_clicked()
{
    this->close();
    /* only called when git push complete successfully without error */
    if(isUploadDone)
    {
        emit pushFinished(isUploadDone);
        return;
    }

    emit pullFinished(isErrorFound);
}

void GitProcessDialog::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == timerActivate)
    {
        this->setFocusPolicy(Qt::StrongFocus);
        this->setFocus();
        this->raise();
        activateWindow();
        killTimer(timerActivate);

//        QMessageBox::information(this, "", "pop");
    }

    QDialog::timerEvent(event);
}
