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
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();
    void FP_templCopy(DATA_BLOB &fp_template);

private:
    Ui::EVDialog *ui;

    DATA_BLOB  m_RegTemplate;   // BLOB that keeps Enrollment Template. It is used to pass it from Enrollment to Verification and also for saving/reading from file.

};

#endif // EVDIALOG_H
