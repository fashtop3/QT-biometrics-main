#ifndef EVDIALOG_H
#define EVDIALOG_H

#include <QDialog>
#include "fedialog.h"
#include "fvdialog.h"

namespace Ui {
class EVDialog;
}

class EVDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EVDialog(QWidget *parent = 0);
    ~EVDialog();

private slots:
    void startVerification(DATA_BLOB fp_RegTemplate);

    void on_pushButtonEnrollment_clicked();

    void on_pushButtonVerification_clicked();

private:
    Ui::EVDialog *ui;

    DATA_BLOB  m_RegTemplate;   // BLOB that keeps Enrollment Template. It is used to pass it from Enrollment to Verification and also for saving/reading from file.
    DATA_BLOB  raw_RegTemplate;
};

#endif // EVDIALOG_H
