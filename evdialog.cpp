#include "evdialog.h"
#include "ui_evdialog.h"

#include <QFile>
#include <QMessageBox>
#include <QtWin>
#include <QDebug>



EVDialog::EVDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EVDialog)
{
    ui->setupUi(this);
    ::ZeroMemory(&m_RegTemplate, sizeof(m_RegTemplate));
    ::ZeroMemory(&raw_RegTemplate, sizeof(raw_RegTemplate));
}

EVDialog::~EVDialog()
{
    delete [] m_RegTemplate.pbData;
    m_RegTemplate.cbData = 0;
    m_RegTemplate.pbData = NULL;

    delete ui;
}


void EVDialog::startVerification(DATA_BLOB fp_RegTemplate)
{
    raw_RegTemplate = fp_RegTemplate;
}

void EVDialog::on_pushButtonEnrollment_clicked()
{
    FEDialog *dialog = new FEDialog(this);
    if(dialog->exec())
    {
        dialog->closeInit();
        dialog->getRawRegTemplate(raw_RegTemplate);
        FVDialog fvdialog;
        fvdialog.verifyAll(raw_RegTemplate);
    }
    else{
        dialog->getRegTemplate(m_RegTemplate);
    }

    delete dialog;
}

void EVDialog::on_pushButtonVerification_clicked()
{
    if (m_RegTemplate.cbData == 0 || m_RegTemplate.pbData == NULL) {
        QMessageBox::critical(this, "Fingerprint Verification",
                              "Before attempting fingerprint verification, you must either select \"Fingerprint Enrollment\" to create a Fingerprint Enrollment Template, or read a previously saved template using \"Read Fingerprint Enrollment Template\".",
                              QMessageBox::Ok|QMessageBox::Escape);
        return;
    }

    FVDialog dialog;
    dialog.loadRegTemplate(m_RegTemplate);
    dialog.exec();//verify(raw_RegTemplate.pbData, raw_RegTemplate.cbData);
}
