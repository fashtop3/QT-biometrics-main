#ifndef FPDATAV_H
#define FPDATAV_H

#include <QObject>
#include "dpDefs.h"
#include "dpRCodes.h"
#include "DPDevClt.h"


class FPDataV : public QObject
{
    Q_OBJECT

    //enum { IDD = IDD_SAMPLEVER_DIALOG };
    enum { WMUS_FP_NOTIFY = WM_USER+1 };

public:
    explicit FPDataV(QObject *parent = 0);
    ~FPDataV();
    void verify(FT_IMAGE_PT pFingerprintImage, int iFingerprintImageSize);
    void setFPData(const DATA_BLOB &data);

    void closeInit();

signals:
    void done(bool isMatchFound);
    void progress(int run, int total);

public slots:
    void loadRegTemplate(const DATA_BLOB &rRegTemplate);
    int verifyAll(const DATA_BLOB &dataBlob);
    void onStartVerification(const DATA_BLOB &blob);
    void onTimeout();

protected:
    void deviceInit();

    DATA_BLOB dataCopy;

private:

    FT_HANDLE      m_fxContext;        // Context for Feature Extraction functions, must be created before calling any of the FT_ functions
    FT_HANDLE      m_mcContext;        // Context for Matching functions, must be created before calling any of the MC_ functions

    BOOL           m_bDoLearning;      // User selects whether to do learning or not via checking checkbox in the dialog
                                       // NOTE: The Enrollment template must be created with Learning allowed
                                       // to use this feature during verification.

    HDPOPERATION   m_hOperationVerify; // Handle of the Operation, must be closed at exit.

    RECT           m_rcDrawArea;       // Rectangle with the desired sizes of the fingerprint image to display

    DATA_BLOB      m_RegTemplate;      // BLOB that keeps Enrollment Template. It is received from the outside (from Enrollment dialog)
    DATA_BLOB  temp_RegTemplate;

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDestroy   (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnFpNotify  (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    bool matchFound;
};

#endif // FPDATAV_H
