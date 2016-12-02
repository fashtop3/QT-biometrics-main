#ifndef EVDIALOG_H
#define EVDIALOG_H

#include <QDialog>
#include <QProcess>
#include "fedialog.h"
#include "fvdialog.h"
#include "xmlreadwrite.h"

namespace Ui {
class EVDialog;
}


class EVDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EVDialog(QWidget *parent = 0);
    ~EVDialog();

public slots:
    void onFileChanged();

private slots:

    void on_pushButtonEnrollment_clicked();

    void on_pushButtonVerification_clicked();

    void onPullFinished(bool isError);
    void onPushFinished(bool isError);

private:
    Ui::EVDialog *ui;

    DATA_BLOB  m_RegTemplate;   // BLOB that keeps Enrollment Template. It is used to pass it from Enrollment to Verification and also for saving/reading from file.
    DATA_BLOB  raw_RegTemplate;
    XmlReadWrite xml;
};

#endif // EVDIALOG_H
