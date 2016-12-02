#ifndef GITPROCESSDIALOG_H
#define GITPROCESSDIALOG_H

#include "evdialog.h"

#include <QDialog>

class QProcess;

namespace Ui {
class GitProcessDialog;
}

class GitProcessDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GitProcessDialog(QWidget *parent = 0);
    ~GitProcessDialog();

   static void fetchUpdates(EVDialog *parent = 0);
   static void pushUpdates(const QString name, const QString id, EVDialog *parent = 0);
    void push(QString arg1, QString arg2);

    void pull();

signals:
    void pullFinished(bool status);
    void pushFinished(bool status);

public slots:
    void onReadyReadStandardOutput();


private slots:
    void on_pushButtonNext_clicked();

private:
    Ui::GitProcessDialog *ui;

    QProcess *process;
    QPalette textEditPallete;
    bool isErrorFound;
    bool isUploadDone;
};

#endif // GITPROCESSDIALOG_H