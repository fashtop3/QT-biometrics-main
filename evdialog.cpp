#include "evdialog.h"
#include "fpdatav.h"
#include "gitprocessdialog.h"
#include "ui_evdialog.h"
#include "verifyworker.h"
#include "xmlreadwrite.h"

#include <QFile>
#include <QMessageBox>
#include <QtWin>
#include <QDebug>
#include <QProcess>
#include <QSettings>
#include <QTimer>
#include <QNetworkRequest>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "fptpath.h"

Q_DECLARE_METATYPE(DATA_BLOB);


EVDialog::EVDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EVDialog)
{
    ui->setupUi(this);
    lockFile = 0;
    /* fill the memory with zeros */
    ::ZeroMemory(&m_RegTemplate, sizeof(m_RegTemplate));
    ::ZeroMemory(&raw_RegTemplate, sizeof(raw_RegTemplate));

    /* set flag to alway show on top */
//    setAttribute(Qt::WA_ShowWithoutActivating);

    fpJsonObject = 0;

    QSettings settings("Dynamic Drive Technology", "DDTFPBiometric");

    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setWindowTitle("Fingerpint Capturing");
    setWindowIcon(QIcon(":/images/icon.jpg"));
}

EVDialog::~EVDialog()
{
    delete ui;
}

//TODO: copy this toimplement verification
//void EVDialog::on_pushButtonVerification_clicked()
//{
//    /* verify if there is a copy of finger template */
//    if (fpJsonObject == 0) {
//        QMessageBox::critical(this, "Fingerprint Verification",
//                              "Before attempting fingerprint verification, you must either select \"Fingerprint Enrollment\" to create a Fingerprint Enrollment Template, or read a previously saved template using \"Read Fingerprint Enrollment Template\".",
//                              QMessageBox::Ok|QMessageBox::Escape);
//        return;
//    }

//    FVDialog dialog;
//    /* Load a copyt template for verification */
//    dialog.loadRegTemplate(fpJsonObject/*m_RegTemplate*/);
//    dialog.exec();
//    fpJsonObject = 0;
//}
