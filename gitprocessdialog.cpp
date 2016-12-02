#include "gitprocessdialog.h"
#include "ui_gitprocessdialog.h"
#include "fptpath.h"

#include <QProcess>
#include <QDebug>
#include <QScrollBar>
#include <QColorDialog>

GitProcessDialog::GitProcessDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GitProcessDialog),
    isErrorFound(false),
    isUploadDone(false)
{
    ui->setupUi(this);
//    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    process = new QProcess(this);
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


    textEditPallete.setColor(QPalette::Base, Qt::black); // set color "Red" for textedit base
    textEditPallete.setColor(QPalette::Text, Qt::green); // set text color which is selected from color pallete
    ui->textEdit->setPalette(textEditPallete); // change textedit palette
}

GitProcessDialog::~GitProcessDialog()
{
    delete ui;
}

void GitProcessDialog::fetchUpdates(EVDialog *parent)
{
    GitProcessDialog processDialog(parent);
    if(parent){
        connect(&processDialog, &GitProcessDialog::pullFinished,
                parent, &parent->onPullFinished);
    }
    processDialog.pull();
    processDialog.exec();
}

void GitProcessDialog::pushUpdates(const QString name, const QString id, EVDialog *parent)
{
    GitProcessDialog processDialog(parent);
    if(parent){
        connect(&processDialog, &GitProcessDialog::pushFinished,
                parent, &parent->onPushFinished);
    }
    processDialog.push(name, id);
    processDialog.exec();
}

void GitProcessDialog::push(QString arg1, QString arg2)
{
    QString batpath = QString(_FPT_PATH_) + "/";
    QString push = QString("git_push.bat \"%1\" \"%2\"")
            .arg(arg1)
            .arg(arg2);

    QString commandStr = QString("cmd.exe") + " /c cd " + batpath + " && " + push;

    process->setWorkingDirectory(batpath);
    process->start(commandStr);
}

void GitProcessDialog::pull()
{
    QString batpath = QString(_FPT_PATH_) + "/";
    QString pull = "git_pull.bat";

    QString commandStr = QString("cmd.exe") + " /c cd " + batpath + " && " + pull;

    process->setWorkingDirectory(batpath);
    process->start(commandStr);

}

void GitProcessDialog::onReadyReadStandardOutput()
{
    static int seek  = 0;
    if(seek == 0)
        ui->textEdit->clear();

    QString stdOutput = process->readAllStandardOutput().data();
    QString stdError = process->readAllStandardError().data();
    if(!stdError.isEmpty()) {
        ui->textEdit->setTextColor(QColor(Qt::red));
        ui->textEdit->append(stdError);
        isErrorFound = true;
        if(stdError.contains("To ssh")) {
            isUploadDone = true;
            ui->pushButtonNext->setText(tr("&Completed"));
            ui->pushButtonNext->setEnabled(true);
        }
    }
    if(!stdOutput.isEmpty()){
        ui->textEdit->setTextColor(QColor(Qt::green));
        ui->textEdit->append(stdOutput);
    }

    QScrollBar *vScroll = ui->textEdit->verticalScrollBar();
    vScroll->setValue(vScroll->maximum());

    ++seek;
}

void GitProcessDialog::on_pushButtonNext_clicked()
{
    this->close();
    if(isUploadDone)
    {
        emit pushFinished(isUploadDone);
        return;
    }

    emit pullFinished(isErrorFound);
}
