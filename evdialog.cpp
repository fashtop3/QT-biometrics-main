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
    setAttribute(Qt::WA_ShowWithoutActivating);
}

EVDialog::~EVDialog()
{
    delete [] m_RegTemplate.pbData;
    m_RegTemplate.cbData = 0;
    m_RegTemplate.pbData = NULL;

    delete ui;
}

/**
 * @brief EVDialog::onFileChanged
 * This method is called when capture.xml file changed
 */
void EVDialog::onFileChanged()
{
    xml.readXML();
    GitProcessDialog::fetchUpdates(this);
}

void EVDialog::on_pushButtonEnrollment_clicked()
{
//    onFileChanged();
    /* Create finger pting verification dialog */
    FEDialog dialog;
    dialog.exec();
    dialog.getRegTemplate(m_RegTemplate);
}

void EVDialog::on_pushButtonVerification_clicked()
{
    /* verify if there is a copy of finger template */
    if (m_RegTemplate.cbData == 0 || m_RegTemplate.pbData == NULL) {
        QMessageBox::critical(this, "Fingerprint Verification",
                              "Before attempting fingerprint verification, you must either select \"Fingerprint Enrollment\" to create a Fingerprint Enrollment Template, or read a previously saved template using \"Read Fingerprint Enrollment Template\".",
                              QMessageBox::Ok|QMessageBox::Escape);
        return;
    }

    FVDialog dialog;
    /* Load a copyt template for verification */
    dialog.loadRegTemplate(m_RegTemplate);
    dialog.exec();
}

void EVDialog::onPullFinished(bool isError)
{
    qDebug() << "Calling pull finished" << isError;

    /* Create finger pting verification dialog */
    FEDialog *dialog = new FEDialog(this);
    dialog->setModal(true);
    if(dialog->exec())
    {
        /* Close the Fingerprint Matching and Acuisition context to allow reinitialion in the FVDialod ctor */
        dialog->closeInit();
        dialog->close();
        /* get copy of raw data sent from the biometric device */
        dialog->getRawRegTemplate(raw_RegTemplate);
        FVDialog fvdialog(this);
        fvdialog.setModal(true);
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

            /* Push all captured fingerprint templates to the remote server */
            GitProcessDialog::pushUpdates(xml.data().value("name"), xml.data().value("id"), this);
        }
    }
    else{ //when exec return false
        /* get the template raedy for OnVerification button clicked */
        dialog->getRegTemplate(m_RegTemplate);
    }

    delete dialog;
}

void EVDialog::onPushFinished(bool isError)
{
    xml.setStatusCode("200");
    xml.setStatus("Fingerprint captured!!!");
    xml.setCaptured("1");
    xml.writeXML();
    QMessageBox::information(this, "Fingerprint enrollment", "Fingerprint capture process completed!!!", QMessageBox::Ok);
    QApplication::closeAllWindows();
}
