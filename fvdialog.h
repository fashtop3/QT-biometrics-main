#ifndef FVDIALOG_H
#define FVDIALOG_H

#include <QDialog>
#include "dpDefs.h"
#include "dpRCodes.h"
#include "DPDevClt.h"

namespace Ui {
class FVDialog;
}

class FVDialog : public QDialog
{
    Q_OBJECT

    //enum { IDD = IDD_SAMPLEVER_DIALOG };
    enum { WMUS_FP_NOTIFY = WM_USER+1 };

public:
    explicit FVDialog(QWidget *parent = 0);
    ~FVDialog();

public slots:
    void LoadRegTemplate(const DATA_BLOB &rRegTemplate);
protected:
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);

private:
    Ui::FVDialog *ui;

    FT_HANDLE      m_fxContext;        // Context for Feature Extraction functions, must be created before calling any of the FT_ functions
    FT_HANDLE      m_mcContext;        // Context for Matching functions, must be created before calling any of the MC_ functions

    BOOL           m_bDoLearning;      // User selects whether to do learning or not via checking checkbox in the dialog
                                       // NOTE: The Enrollment template must be created with Learning allowed
                                       // to use this feature during verification.

    HDPOPERATION   m_hOperationVerify; // Handle of the Operation, must be closed at exit.

    RECT           m_rcDrawArea;       // Rectangle with the desired sizes of the fingerprint image to display

    DATA_BLOB      m_RegTemplate;      // BLOB that keeps Enrollment Template. It is received from the outside (from Enrollment dialog)

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDestroy   (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnFpNotify  (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    QWidget h_bmpWidget;

    void displayFingerprintImage(const DATA_BLOB* pFingerprintImage);//FT_IMAGE_PT pFingerprintImage);
    void verify(FT_IMAGE_PT pFingerprintImage, int iFingerprintImageSize);
    void addStatus(const QString &status);
};

#endif // FVDIALOG_H
