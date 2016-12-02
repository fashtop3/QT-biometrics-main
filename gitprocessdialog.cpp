#include "gitprocessdialog.h"
#include "ui_gitprocessdialog.h"
#include "fptpath.h"

#include <QProcess>
#include <QDebug>

GitProcessDialog::GitProcessDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GitProcessDialog)
{
    ui->setupUi(this);
//    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
}

GitProcessDialog::~GitProcessDialog()
{
    delete ui;
}

void GitProcessDialog::fetchUpdates()
{
    QString batpath = QString(_FPT_PATH_) + "/";
    QString pull = "git_pull.bat";

    QProcess process;
    QFileInfo cmdFile( "C:\\Windows\\system32\\cmd.exe");
    if(!cmdFile.exists())
    {
        //output message here
        return ;
    }

    QString commandStr = cmdFile.absoluteFilePath() +
            " /c cd " + batpath + " && " + pull;

    process.execute(commandStr);
    if (process.waitForStarted(120000))
    {
        qDebug("Process started");
        process.waitForFinished();
        qDebug() << process.readAllStandardOutput();
    }
    else {
        qDebug() << "Failed to start";
    }

}

void GitProcessDialog::pushUpdates(QString arg1, QString arg2)
{
    QString batpath = QString(_FPT_PATH_) + "/";
    QString push = QString("git_push.bat \"%1\" \"%2\"")
            .arg(arg1)
            .arg(arg2);

    QProcess process;
    QFileInfo cmdFile( "C:\\Windows\\system32\\cmd.exe");

    QString commandStr = cmdFile.absoluteFilePath() +
            " /c cd " + batpath + " && " + push;
    qDebug() << commandStr;
    process.execute(commandStr);
    if (process.waitForStarted(120000))
    {
        qDebug("Process started");
        process.waitForFinished();
        qDebug() << process.readAllStandardOutput();
    }
    else {
        qDebug() << "Failed to start";
    }
}
