#ifndef FEDIALOG_H
#define FEDIALOG_H

#include <QDialog>
#include "dpDefs.h"
#include "dpRCodes.h"
#include "DPDevClt.h"


namespace Ui {
class FEDialog;
}

class FEDialog : public QDialog
{
    Q_OBJECT

    enum { WMUS_FP_NOTIFY = WM_USER+1 };

public:
    explicit FEDialog(QWidget *parent = 0);
    ~FEDialog();

signals:
    void onVerificationStart(DATA_BLOB fp_RegTemplate);

public slots:
    void getRegTemplate(DATA_BLOB &rRegTemplate) const;
    void closeInit();

    void initContext();

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);
    void displayImage(const DATA_BLOB* pImageBlob);
    void addStatus(QString status);

    void addToEnroll(FT_IMAGE_PT pFingerprintImage, int iFingerprintImageSize);
    bool saveTemplate();
    bool saveFile(const QString &fileName);
    bool writeFile(const QString &fileName);
    void closAcquisitionAndContext();
private:
    Ui::FEDialog *ui;
    FT_HANDLE   m_fxContext;
    FT_HANDLE   m_mcContext;
    int         m_NumberOfPreRegFeatures; // Number of the pre-Enrollment fingerprint templates needed to create one Enrollment template.
    FT_BYTE**   m_TemplateArray;          // Array that keeps pre-Enrollment fingerprint templates. It is allocated with the size equal to number of pre-Enrollment templetes to create a Enrollment template (usually 4)
    int         m_nRegFingerprint;        // Pre-Enrollment number, index in the above array.
    RECT        m_rcDrawArea;             // Rectangle with the desired sizes of the fingerprint image to display
    HDPOPERATION   m_hOperationEnroll;     // Handle of the Operation, must be closed at exit.
    FT_REG_OPTIONS m_mcRegOptions;           // Enrollment options. Collected from check boxes of the dialog.
    DATA_BLOB      m_RegTemplate;            // BLOB that keeps Enrollment Template.
    QString curFile;
    enum { MagicNumber = 0x7F51C883};
    QWidget h_bmpWidget;
    bool hasReadyTemplate;
};

#endif // FEDIALOG_H
