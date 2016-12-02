#include "evdialog.h"
#include "gitprocessdialog.h"
#include "ui_evdialog.h"
#include "xmlreadwrite.h"

#include <QFile>
#include <QMessageBox>
#include <QtWin>
#include <QDebug>
#include <QProcess>
#include "fptpath.h"


EVDialog::EVDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EVDialog)
{
    ui->setupUi(this);
    /* fill the memory with zeros */
    ::ZeroMemory(&m_RegTemplate, sizeof(m_RegTemplate));
    ::ZeroMemory(&raw_RegTemplate, sizeof(raw_RegTemplate));

    /* set flag to alway show on top */
//    setWindowFlags(Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_ShowWithoutActivating);
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
    XmlReadWrite xml;
    xml.readXML();

    GitProcessDialog gitDlg(this);
    gitDlg.setModal(true);
    gitDlg.exec();

    /* Create finger pting verification dialog */
    FEDialog *dialog = new FEDialog(this);
    if(dialog->exec())
    {
        /* Close the Fingerprint Matching and Acuisition context to allow reinitialion in the FVDialod ctor */
        dialog->closeInit();
        /* get copy of raw data sent from the biometric device */
        dialog->getRawRegTemplate(raw_RegTemplate);
        FVDialog fvdialog;
        /* pass the raw template to verify with all existing saved .fpt files*/
        if(fvdialog.verifyAll(raw_RegTemplate))
        {
            xml.setStatusCode("401");
            xml.setStatus("Fingerprint already exists!!!!");
            xml.setCaptured("0");
            xml.writeXML();
            QMessageBox::critical(this, "Fingerprint verification", "Process terminated!!! Fingerprint match found!.", QMessageBox::Close);
        }
        else{
            //copy the file from temp ../ with the req ID
            QString newName = _FPT_PATH_ + QString(QDir::separator()) + "_" + xml.data().value("id") + "_.fpt";
            QFile::rename(_TEMP_FPT_PATH("temp.fpt"), newName);

            //TODO: push updates process

            //show process windows window for updating the remote server
            //block other process
            //write success response
            //emit close of dialog;
        }
    }
    else{ //when exec return false
        /* get the template raedy for OnVerification button clicked */
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
    /* Load a copyt template for verification */
    dialog.loadRegTemplate(m_RegTemplate);
    dialog.exec();//verify(raw_RegTemplate.pbData, raw_RegTemplate.cbData);
}
