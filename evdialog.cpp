#include "evdialog.h"
#include "ui_evdialog.h"

#include <QMessageBox>




EVDialog::EVDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EVDialog)
{
    ui->setupUi(this);
    ::ZeroMemory(&m_RegTemplate, sizeof(m_RegTemplate));
}

EVDialog::~EVDialog()
{
    delete [] m_RegTemplate.pbData;
    m_RegTemplate.cbData = 0;
    m_RegTemplate.pbData = NULL;

    delete ui;
}

void EVDialog::on_pushButton_clicked()
{
    FEDialog dialog;
//    connect(&dialog, SIGNAL(FP_templGenerated(DATA_BLOB&)), this, SLOT(FP_templCopy(DATA_BLOB&)));
    dialog.exec();
    dialog.getRegTemplate(m_RegTemplate);
}

void EVDialog::on_pushButton_2_clicked()
{
    if (m_RegTemplate.cbData == 0 || m_RegTemplate.pbData == NULL) {
        QMessageBox::critical(this, "Fingerprint Verification",
                              "Before attempting fingerprint verification, you must either select \"Fingerprint Enrollment\" to create a Fingerprint Enrollment Template, or read a previously saved template using \"Read Fingerprint Enrollment Template\".",
                              QMessageBox::Ok|QMessageBox::Escape);
        return;
    }
    FVDialog dialog;
    dialog.LoadRegTemplate(m_RegTemplate);
    dialog.exec();
}

void EVDialog::FP_templCopy(DATA_BLOB &fp_template)
{
    m_RegTemplate = fp_template;
}
