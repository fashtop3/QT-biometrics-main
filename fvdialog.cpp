#include "fvdialog.h"
#include "ui_fvdialog.h"
#include <stdio.h>
#include <QtWin>
#include <comdef.h>

#include "dpFtrEx.h"
#include "dpMatch.h"


#include <QThread>
#include <QDebug>
#include <QFileInfoList>
#include <QMessageBox>
#include <QScrollBar>
#include <QDir>

#include "xmlreadwrite.h"
#include "fptpath.h"

//const GUID GUID_NULL = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };

FVDialog::FVDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FVDialog),
    m_bDoLearning(FT_FALSE),
    m_hOperationVerify(0),
    m_fxContext(0),
    m_mcContext(0),
    matchFound(false)
{
    ui->setupUi(this);

    deviceInit();

    textEditPalete.setColor(QPalette::Base, Qt::black); // set color "Red" for textedit base
    textEditPalete.setColor(QPalette::Text, Qt::white); // set text color which is selected from color pallete
    ui->textEdit->setPalette(textEditPalete);
    setWindowIcon(QIcon(":/images/icon.jpg"));
}

FVDialog::~FVDialog()
{
    if (m_hOperationVerify) {
        DPFPStopAcquisition(m_hOperationVerify);    // No error checking - what can we do at the end anyway?
        DPFPDestroyAcquisition(m_hOperationVerify);
        m_hOperationVerify = 0;
    }

    if (m_fxContext) {
        FX_closeContext(m_fxContext);
        m_fxContext = 0;
    }

    if (m_mcContext) {
        MC_closeContext(m_mcContext);
        m_mcContext = 0;
    }

    delete [] m_RegTemplate.pbData;
    m_RegTemplate.cbData = 0;
    m_RegTemplate.pbData = NULL;

    delete ui;
}

void FVDialog::loadRegTemplate(const QJsonObject* fpJsonObj) {
    // Delete the old stuff that may be in the template.
    delete [] m_RegTemplate.pbData;
    m_RegTemplate.pbData = NULL;
    m_RegTemplate.cbData = 0;

    /**
     * @brief pdata:: convert Json string to bytearray and fromBase64
     */
    QByteArray pdata = QByteArray::fromBase64(fpJsonObj->value("pbdata").toVariant().toByteArray());
    m_RegTemplate.pbData = new BYTE[fpJsonObj->value("cbdata").toInt()];
    if (!m_RegTemplate.pbData) _com_issue_error(E_OUTOFMEMORY);
    ::CopyMemory(m_RegTemplate.pbData, (BYTE *) pdata.data() , fpJsonObj->value("cbdata").toInt());
    m_RegTemplate.cbData = fpJsonObj->value("cbdata").toInt();
}


bool FVDialog::nativeEvent(const QByteArray &eventType, void *message, long *result)
{

    Q_UNUSED(result);
    Q_UNUSED(eventType);

    MSG *msg = static_cast<MSG*> (message);
    if(msg->message != 1025) return 0;

     ui->textEdit->setTextColor(QColor(Qt::white));

    switch(msg->wParam) {
        case WN_COMPLETED: {
            addStatus("Fingerprint image captured");

            DATA_BLOB* pImageBlob = reinterpret_cast<DATA_BLOB*>(msg->lParam);

            // Display fingerprint image
            displayFingerprintImage(pImageBlob);//->pbData);

            // Match the new fingerprint image and Enrollment template
            verify(pImageBlob->pbData, pImageBlob->cbData);
            break;
        }
        case WN_ERROR: {
            char buffer[101] = {0};
            snprintf(buffer, 100, "Error happened. Error code: 0x%X", msg->lParam);
            addStatus(buffer);
            break;
        }
        case WN_DISCONNECT:
            ui->textEdit->setTextColor(QColor(Qt::red));
            addStatus("Fingerprint reader disconnected");
            break;
        case WN_RECONNECT:
            ui->textEdit->setTextColor(QColor(Qt::green));
            addStatus("Fingerprint reader connected");
            break;
        case WN_FINGER_TOUCHED:
            ui->textEdit->setTextColor(QColor(Qt::darkYellow));
            addStatus("Finger touched");
            break;
        case WN_FINGER_GONE:
            ui->textEdit->setTextColor(QColor(Qt::magenta));
            addStatus("Finger gone");
            break;
        case WN_IMAGE_READY:
            ui->textEdit->setTextColor(QColor(Qt::darkYellow));
            addStatus("Fingerprint image ready");
            break;
        case WN_OPERATION_STOPPED:
            ui->textEdit->setTextColor(QColor(Qt::red));
            addStatus("Fingerprint Verification Operation stopped");
            break;
    }
    return 0;

}

DPFP_STDAPI DPFPConvertSampleToBitmap(const DATA_BLOB* pSample, PBYTE pBitmap, size_t* pSize);
void FVDialog::displayFingerprintImage(const DATA_BLOB *pFingerprintImage)
{
    HRESULT hr = S_OK;
    PBITMAPINFO pOutBmp = NULL;
    try {
        size_t Size = 0;
        hr = DPFPConvertSampleToBitmap(pFingerprintImage, 0, &Size);
        pOutBmp = (PBITMAPINFO)new BYTE[Size];
        hr = DPFPConvertSampleToBitmap(pFingerprintImage, (PBYTE)pOutBmp, &Size);

        if (SUCCEEDED(hr)) {
            size_t dwColorsSize = pOutBmp->bmiHeader.biClrUsed * sizeof(PALETTEENTRY);
            const BYTE* pBmpBits = (PBYTE)pOutBmp + sizeof(BITMAPINFOHEADER) + dwColorsSize;

            // Create bitmap and set its handle to the control for display.

            h_bmpWidget.resize(ui->labelImage->size());
            HDC hdcScreen = GetDC((HWND) h_bmpWidget.winId());
            HDC hdcMem = CreateCompatibleDC(hdcScreen);

            HBITMAP hBmp = CreateCompatibleBitmap(hdcScreen, ui->labelImage->width(), ui->labelImage->height());
            SelectObject(hdcMem, hBmp);


            int i = StretchDIBits(hdcMem, 0, 0, h_bmpWidget.width(), h_bmpWidget.height(), 0, 0, pOutBmp->bmiHeader.biWidth, pOutBmp->bmiHeader.biHeight, pBmpBits, pOutBmp, DIB_RGB_COLORS, SRCCOPY);
            int j = BitBlt(hdcScreen, 0, 0, h_bmpWidget.width(), h_bmpWidget.height(), hdcMem, 0, 0, SRCCOPY);

            /* produce image here */
            QPixmap pixmap = QtWin::fromHBITMAP(hBmp);
            ui->labelImage->setPixmap(pixmap);
//            pixmap.toImage().save("newImage.bmp");

            DeleteDC(hdcMem);
            ReleaseDC((HWND)h_bmpWidget.winId(), hdcScreen);

        }
        delete [] (PBYTE)pOutBmp;
    }
    catch(_com_error E) {
        hr = E.Error();
        qDebug("exception found 1");
    }
    catch(...) {
        hr = E_UNEXPECTED;
        qDebug("exception found 2");
    }

}


void FVDialog::verify(FT_IMAGE_PT pFingerprintImage, int iFingerprintImageSize) {
    HRESULT hr = S_OK;
    matchFound = false;
    FT_BYTE* pVerTemplate = NULL;
    ui->textEdit->setTextColor(QColor(Qt::white));

    try {
        FT_RETCODE rc = FT_OK;

        // Get recommended length of the Pre-Enrollment feature sets.
        // It actually could be done once at the beginning, but this is just sample code...
        int iRecommendedVerFtrLen = 0;
        rc = FX_getFeaturesLen(FT_VER_FTR,
                               &iRecommendedVerFtrLen,
                               NULL); // Minimal Fingerprint feature set Length - do not use if possible
        if (FT_OK != rc) _com_issue_error(E_FAIL);

        // Extract Features (i.e. create fingerprint template)
        FT_IMG_QUALITY imgQuality;
        FT_FTR_QUALITY ftrQuality;
        FT_BOOL bEextractOK = FT_FALSE;

        if (NULL == (pVerTemplate = new FT_BYTE[iRecommendedVerFtrLen])) _com_issue_error(E_OUTOFMEMORY);
        rc = FX_extractFeatures(m_fxContext,             // Context of Feature Extraction
                                iFingerprintImageSize,   // Size of the fingerprint image
                                pFingerprintImage,       // Pointer to the image buffer
                                FT_VER_FTR,              // Requested Verification Features
                                iRecommendedVerFtrLen,   // Length of the Features(template) buffer received previously from the SDK
                                pVerTemplate,            // Pointer to the buffer where the template to be stored
                                &imgQuality,             // Returned quality of the image. If feature extraction fails because used did not put finger on the reader well enough, this parameter be used to tell the user how to put the finger on the reader better. See FT_IMG_QUALITY enumeration.
                                &ftrQuality,             // Returned quality of the features. It may happen that the fingerprint of the user cannot be used. See FT_FTR_QUALITY enumeration.
                                &bEextractOK);           // Returned result of Feature extraction. FT_TRUE means extracted OK.

        if (FT_OK <= rc && bEextractOK == FT_TRUE) {
            // Features extracted OK (i.e. fingerprint Verification template was created successfully)
            // Now match this template and the Enrollment template.
            FT_BOOL bRegFeaturesChanged = FT_FALSE;
            FT_VER_SCORE VerScore = FT_UNKNOWN_SCORE;
            double dFalseAcceptProbability = 0.0;
            FT_BOOL bVerified = FT_FALSE;

            rc = MC_verifyFeaturesEx(m_mcContext,              // Matching Context
                                     m_RegTemplate.cbData,     // Pointer to the Enrollment template content
                                     m_RegTemplate.pbData,     // Length of the Enrollment template
                                     iRecommendedVerFtrLen,    // Length of the Verification template
                                     pVerTemplate,             // Pointer to the Verification template content
                                     m_bDoLearning,            // Whether the Learning is desired - got it from checkbox in the dialog
                                     &bRegFeaturesChanged,     // Out: Whether the Learning actually happened
                                     NULL,                     // Reserved, must be NULL
                                     &VerScore,                // Out: Matching score, see score enumeration FT_VER_SCORE
                                     &dFalseAcceptProbability, // Out: Probability to falsely match these fingerprints
                                     &bVerified);              // Returned result of fingerprint match. FT_TRUE means fingerprints matched.
            if (FT_OK <= rc) {
                if (FT_OK != rc) {
                    char buffer[101] = {0};
                    ULONG uSize = 100;
                    snprintf(buffer, uSize, "Warning: %ld (see dpRCodes.h)", rc);
                    ui->textEdit->setTextColor(QColor(Qt::red));
                    addStatus(buffer);
                }

                if (bVerified == FT_TRUE) {
                    ui->textEdit->setTextColor(QColor(Qt::green));
                    addStatus("Fingerprint Matches!");
                    ui->lineEditPrompt->setText("Scan another finger to run verification again.");
                    char buffer[101] = {0};
                    ULONG uSize = 100;
                    snprintf(buffer, uSize, "%lf", dFalseAcceptProbability);
                    ui->lineEditProbability->setText(buffer);
                    matchFound = true;
                    qDebug("Fingerprint Matches!");
                }
                else {
                    ui->textEdit->setTextColor(QColor(Qt::red));
                    addStatus("Fingerprint did not Match!");
                    ui->lineEditPrompt->setText("Scan another finger to run verification again.");
                    char buffer[101] = {0};
                    ULONG uSize = 100;
                    snprintf(buffer, uSize, "%lf", dFalseAcceptProbability);
                    ui->lineEditProbability->setText(buffer);
                    matchFound = false;
                    qDebug("Fingerprint did not Match!");
                }
            }
            else {
                char buffer[101] = {0};
                ULONG uSize = 100;
                snprintf(buffer, uSize, "Verification Operation failed, Error: %ld.", rc);
                QMessageBox::critical(this, "Fingerprint Verification", buffer, QMessageBox::Ok | QMessageBox::Escape);
                 ui->lineEditPrompt->setText("Scan your finger for verification again.");
                 matchFound = false;
                 qDebug("Fingerprint Verification.");
            }
        }
    }
    catch(_com_error E) {
        hr = E.Error();
    }
    catch(...) {
        hr = E_UNEXPECTED;
    }
    delete [] pVerTemplate;
}

void FVDialog::addStatus(const QString &status)
{
    ui->textEdit->append(status);
    QScrollBar *sb = ui->textEdit->verticalScrollBar();
    sb->setValue(sb->maximum());
}

int FVDialog::verifyAll(const DATA_BLOB& dataBlob)
{
    matchFound = false;
    /* open the dir */
    qDebug() << _FPT_PATH_();
    QDir dir(_FPT_PATH_());
    /* Filter out only .fpt files*/
    dir.setFilter(QDir::Files|QDir::NoDotAndDotDot);
    QFileInfoList files = dir.entryInfoList(QStringList() << "*.fpt");


    /* iterate the file names in the string list*/
    QListIterator<QFileInfo> i(files);
    while(i.hasNext()) {
        DATA_BLOB fpData;
        QFileInfo fileInfo = i.next(); //get the filename
        QString fileName = fileInfo.absoluteFilePath();
        QFile fpFile(fileName);
        if(!fpFile.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, "Fingerprint verification", fpFile.errorString(), QMessageBox::Close);
            return -1;
        }
        DWORD dwSize = fpFile.size();
        DWORD dwNumRead = 0;
        BYTE buffer[dwSize];
        if(!buffer) {
            #ifdef QT_DEBUG
                qDebug("out of memory");
            #endif
            return -1;
        }

        QDataStream out(&fpFile);
        if(dwNumRead = out.readRawData((char*)buffer, dwSize)){
            //BUG: delete [] m_RegTemplate.pbData;
            m_RegTemplate.pbData = {0};

            m_RegTemplate.pbData = buffer;
            m_RegTemplate.cbData = dwNumRead;
            #ifdef QT_DEBUG
                qDebug() << "data read: " << dwNumRead;
            #endif
        }

        /* calling verification method with the features gotten from Enrolling Diaoolog
            and implicitly matches it with all the .fpt s'
        */
        verify(dataBlob.pbData, dataBlob.cbData);
        if(matchFound)
        {
            break; //if match is found break the loop and return the match status
        }
    }

    return matchFound;
}

void FVDialog::onTimeout()
{
    qDebug() << "from the verify thread" << QThread::currentThreadId();
}


void FVDialog::deviceInit()
{
    HRESULT hr = S_OK;
    try {
        FT_RETCODE rc = FT_OK;

        // Create Context for Feature Extraction
        if (FT_OK != (rc = FX_createContext(&m_fxContext))) {
            QMessageBox::critical(this, "Fingerprint Verification", "Cannot create Feature Extraction Context.",
                                  QMessageBox::Close|QMessageBox::Escape);
            this->close();
            return;  // return TRUE  unless you set the focus to a control
        }

        // Create Context for Matching
        if (FT_OK != (rc = MC_createContext(&m_mcContext))) {
            QMessageBox::critical(this, "Fingerprint Verification", "Cannot create Matching Context.",
                                  QMessageBox::Close|QMessageBox::Escape);
            this->close();
            return;  // return TRUE  unless you set the focus to a control
        }


        // Start Verification.
        DP_ACQUISITION_PRIORITY ePriority = DP_PRIORITY_NORMAL; // Using Normal Priority, i.e. fingerprint will be sent to
                                              // this process only if it has active window on the desktop.
        HRESULT re;
        HWND m_hWnd = (HWND)this->winId();
        if(S_OK != (re = DPFPCreateAcquisition(ePriority, GUID_NULL, DP_SAMPLE_TYPE_IMAGE, m_hWnd, WMUS_FP_NOTIFY, &m_hOperationVerify)))
        {
            if(re == E_ACCESSDENIED)
            {
                throw -5;
            }

            if(re == E_INVALIDARG)
            {
                throw -6;
            }
        }
        if(S_OK != DPFPStartAcquisition(m_hOperationVerify))
        {
            throw -7;
        }

        ui->lineEditPrompt->setText("Scan your finger for verification.");
    }
    catch(_com_error& E) {
        hr = E.Error();
    }
    catch(...) {
        hr = E_UNEXPECTED;
    }

    if (FAILED(hr)) {
        ui->lineEditPrompt->setText("Error happened");
        QMessageBox::critical(this, "Fingerprint Verification", "Verification error", QMessageBox::Close|QMessageBox::Escape);
        this->close();
    }
}
