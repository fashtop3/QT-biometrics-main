#include "fedialog.h"
#include "ui_fedialog.h"
#include <minwinbase.h>
#include <QMessageBox>
#include <QFileDialog>
#include <QScrollBar>
#include <comdef.h>
#include <stdio.h>
#include <QDebug>
#include "dpFtrEx.h"
#include "dpMatch.h"
#include <QtWin>
#include <windows.h>
#include <qt_windows.h>



#include <QBuffer>
#include <QDateTime>
#include <iostream>
#include "fvdialog.h"
#include "resource.h"
#include "fptpath.h"

const GUID GUID_NULL = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };

FEDialog::FEDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FEDialog),
    m_hOperationEnroll(0),
    m_fxContext(0),
    m_mcContext(0),
    m_TemplateArray(NULL),
    m_nRegFingerprint(0),
    m_mcRegOptions(FT_DEFAULT_REG),
    hasReadyTemplate(false)
{
    ui->setupUi(this);

    ::ZeroMemory(&m_RawRegTemplate, sizeof(m_RawRegTemplate));

    /* Initiallize device to the context */
    initContext();

    ui->pushButtonNext->setEnabled(false);
    ui->progressBar->setVisible(0);
    ui->progressBar->setRange(0,100);
    ui->progressBar->setValue(0);

    /* change the text edit look with palette */
    textEditPalete.setColor(QPalette::Base, Qt::black); // set color "Red" for textedit base
    textEditPalete.setColor(QPalette::Text, Qt::white); // set text color which is selected from color pallete
    ui->textEdit->setPalette(textEditPalete);
    setWindowIcon(QIcon(":/images/icon.jpg"));
}

/**
 * @brief FEDialog::closeInit
 * @attention Closes the Acquisition, FX_closeContext and MC_closeContext
 * temporarily
 */
void FEDialog::closeInit()
{
    if (m_hOperationEnroll) {
        DPFPStopAcquisition(m_hOperationEnroll);    // No error checking - what can we do at the end anyway?
        DPFPDestroyAcquisition(m_hOperationEnroll);
        m_hOperationEnroll = 0;
    }

    if (m_fxContext) {
        FX_closeContext(m_fxContext);
        m_fxContext = 0;
    }

    if (m_mcContext) {
        MC_closeContext(m_mcContext);
        m_mcContext = 0;
    }
}

FEDialog::~FEDialog()
{
    closeInit();

    if(m_TemplateArray){
        for (int i=0; i<m_NumberOfPreRegFeatures; ++i)
            if(m_TemplateArray[i]) delete [] m_TemplateArray[i], m_TemplateArray[i] = NULL; // Delete Pre-Enrollment feature sets stored in the array.

        delete [] m_TemplateArray; // delete the array itself.
        m_TemplateArray = NULL;
    }

    delete ui;

    m_RegTemplate.pbData = {0};
    m_RegTemplate.cbData = 0;
    m_RegTemplate.pbData = NULL;
}

const bool FEDialog::isTemplateReady() const
{
    return hasReadyTemplate;
}

QPushButton *FEDialog::nextButtonPtr()
{
    return ui->pushButtonNext;
}

QProgressBar *FEDialog::progressBarPtr()
{
    return ui->progressBar;
}

bool FEDialog::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(result);
    Q_UNUSED(eventType);

    MSG *msg = static_cast<MSG*> (message);
    if(msg->message != 1025) return 0;

    ui->textEdit->setTextColor(QColor(Qt::white));
    switch (msg->wParam) {
        case WN_COMPLETED: {
            //display message on the plaintext
#ifdef QT_DEBUG
        qDebug("Fingerprint image captured");
#endif
            DATA_BLOB* pImageBlob = reinterpret_cast<DATA_BLOB*>(msg->lParam);

            // Display fingerprint image
            displayImage(pImageBlob);

            // Add fingerprint image to the Enrollment
            addToEnroll(pImageBlob->pbData, pImageBlob->cbData);
            break;
        }
        case WN_ERROR: {
            char buffer[101] = {0};
            snprintf(buffer, 100, "Error happened. Error code: 0x%X", msg->lParam);
#ifdef QT_DEBUG
            qDebug(buffer);
#endif
            addStatus(buffer);
             break;
        }
        case WN_DISCONNECT: {
            qDebug("Fingerprint reader disconnected");
            ui->textEdit->setTextColor(QColor(Qt::red));
            addStatus("Fingerprint reader disconnected");
            break;
        }
        case WN_RECONNECT: {
#ifdef QT_DEBUG
         qDebug("Fingerprint reader connected");
#endif
            ui->textEdit->setTextColor(QColor(Qt::green));
            addStatus("Fingerprint reader connected");
            break;
        }
        case WN_FINGER_TOUCHED:
#ifdef QT_DEBUG
        qDebug("Finger touched");
#endif
            ui->textEdit->setTextColor(QColor(Qt::darkYellow));
            addStatus("Finger touched");
            break;
        case WN_FINGER_GONE:
#ifdef QT_DEBUG
        qDebug("Finger gone");
#endif
            ui->textEdit->setTextColor(QColor(Qt::magenta));
            addStatus("Finger gone");
            break;
        case WN_IMAGE_READY:
#ifdef QT_DEBUG
        qDebug("Fingerprint image ready");
#endif
            ui->textEdit->setTextColor(QColor(Qt::darkYellow));
            addStatus("Fingerprint image ready");
            break;
        case WN_OPERATION_STOPPED:
#ifdef QT_DEBUG
        qDebug("Fingerprint Enrollment Operation stopped");
#endif
            break;
    }

    return 0;
}

DPFP_STDAPI DPFPConvertSampleToBitmap(const DATA_BLOB* pSample, PBYTE pBitmap, size_t* pSize);

void FEDialog::displayImage(const DATA_BLOB *pImageBlob)
{
    HRESULT hr = S_OK;
    PBITMAPINFO pOutBmp = NULL;
    try {
        size_t Size = 0;
        hr = DPFPConvertSampleToBitmap(pImageBlob, 0, &Size);
        pOutBmp = (PBITMAPINFO)new BYTE[Size];
        hr = DPFPConvertSampleToBitmap(pImageBlob, (PBYTE)pOutBmp, &Size);

        if (SUCCEEDED(hr)) {
            size_t dwColorsSize = pOutBmp->bmiHeader.biClrUsed * sizeof(PALETTEENTRY);
            const BYTE* pBmpBits = (PBYTE)pOutBmp + sizeof(BITMAPINFOHEADER) + dwColorsSize;

            /* Create bitmap and set its handle to the control for display. */

            h_bmpWidget.resize(ui->labelImage->size());
            HDC hdcScreen = GetDC((HWND) h_bmpWidget.winId());
            HDC hdcMem = CreateCompatibleDC(hdcScreen);

            HBITMAP hBmp = CreateCompatibleBitmap(hdcScreen, ui->labelImage->width(), ui->labelImage->height());
            SelectObject(hdcMem, hBmp);


            int i = StretchDIBits(hdcMem, 0, 0, h_bmpWidget.width(), h_bmpWidget.height(), 0, 0, pOutBmp->bmiHeader.biWidth, pOutBmp->bmiHeader.biHeight, pBmpBits, pOutBmp, DIB_RGB_COLORS, SRCCOPY);
            int j = BitBlt(hdcScreen, 0, 0, h_bmpWidget.width(), h_bmpWidget.height(), hdcMem, 0, 0, SRCCOPY);

            /* Create image from a bitmap to jpg format */
            QPixmap pixmap = QtWin::fromHBITMAP(hBmp);
            ui->labelImage->setPixmap(pixmap);
//            pixmap.toImage().save(_TEMP_FPT_PATH("temp.jpg"));

            DeleteDC(hdcMem);
            ReleaseDC((HWND)h_bmpWidget.winId(), hdcScreen);

        }
        delete [] (PBYTE)pOutBmp;
    }
    catch(_com_error E) {
        hr = E.Error();
#ifdef QT_DEBUG
        qDebug("exception found 1");
#endif
    }
    catch(...) {
        hr = E_UNEXPECTED;
#ifdef QT_DEBUG
        qDebug("exception found 2");
#endif
    }
}

void FEDialog::addStatus(QString status)
{
    ui->textEdit->append(status);
    QScrollBar *sb = ui->textEdit->verticalScrollBar();
    sb->setValue(sb->maximum());
}

/**
 * @brief FEDialog::addToEnroll
 * @param pFingerprintImage
 * @param iFingerprintImageSize
 * \brief Create new template for enrollment
 */
void FEDialog::addToEnroll(FT_IMAGE_PT pFingerprintImage, int iFingerprintImageSize)
{
    HRESULT hr = S_OK;
    FT_BYTE* pPreRegTemplate = NULL;
    FT_BYTE* pRegTemplate = NULL;

    ui->textEdit->setTextColor(QColor(Qt::white));

    try {
        if (m_nRegFingerprint < m_NumberOfPreRegFeatures) { // Do not generate more Pre-Enrollment feature sets than needed
            FT_RETCODE rc = FT_OK;

            // Get recommended length of the Pre-Enrollment Fingerprint feature sets.
            // It actually could be done once at the beginning, but this is just sample code...
            int iRecommendedPreRegFtrLen = 0;
            rc = FX_getFeaturesLen(FT_PRE_REG_FTR,
                                   &iRecommendedPreRegFtrLen,
                                   NULL); // Minimal Fingerprint feature set Length - do not use if possible
            if (FT_OK != rc) _com_issue_error(E_FAIL);

            // Extract Features (i.e. create fingerprint template)
            FT_IMG_QUALITY imgQuality;
            FT_FTR_QUALITY ftrQuality;
            FT_BOOL bEextractOK = FT_FALSE;

            if (NULL == (pPreRegTemplate = new FT_BYTE[iRecommendedPreRegFtrLen])) _com_issue_error(E_OUTOFMEMORY);
            rc = FX_extractFeatures(m_fxContext,              // Context of Feature Extraction
                                    iFingerprintImageSize,    // Size of the fingerprint image
                                    pFingerprintImage,        // Pointer to the image buffer
                                    FT_PRE_REG_FTR,           // Requested Pre-Enrollment Features
                                    iRecommendedPreRegFtrLen, // Length of the Features(template) buffer received previously from the SDK
                                    pPreRegTemplate,          // Pointer to the buffer where the template to be stored
                                    &imgQuality,              // Returned quality of the image. If feature extraction fails because used did not put finger on the reader well enough, this parameter be used to tell the user how to put the finger on the reader better. See FT_IMG_QUALITY enumeration.
                                    &ftrQuality,              // Returned quality of the features. It may happen that the fingerprint of the user cannot be used. See FT_FTR_QUALITY enumeration.
                                    &bEextractOK);            // Returned result of Feature extraction. FT_TRUE means extracted OK.

            // If feature extraction succeeded, add the pre-Enrollment feature sets
            // to the set of 4 templates needed to create Enrollment template.
            if (FT_OK <= rc && bEextractOK == FT_TRUE) {
                /* output status message*/
                ui->textEdit->setTextColor(QColor(Qt::yellow));
                addStatus("Pre-Enrollment feature set generated successfully");

                m_TemplateArray[m_nRegFingerprint++] = pPreRegTemplate;
                pPreRegTemplate = NULL;   // This prevents deleting at the end of the function

                // Display current fingerprint image number and total required number of images.
                TCHAR buffer[101] = {0};
                ULONG uSize = 100;

                snprintf((LPSTR)buffer, uSize, "%ld", m_nRegFingerprint);

                ui->lineEditCount->setText((LPSTR)buffer);

                snprintf((LPSTR)buffer, uSize, "%ld", m_NumberOfPreRegFeatures);

                ui->lineEditMax->setText((LPSTR)buffer);

                if (m_nRegFingerprint == m_NumberOfPreRegFeatures)
                {   // We collected enough Pre-Enrollment feature sets, create Enrollment template out of them.
                    // Get the recommended length of the fingerprint template (features).
                    // It actually could be done once at the beginning, but this is just sample code...
                    int iRecommendedRegFtrLen = 0;
                    rc = MC_getFeaturesLen(FT_REG_FTR,
                                           m_mcRegOptions,
                                           &iRecommendedRegFtrLen,
                                           NULL);

                    if (FT_OK != rc) _com_issue_error(E_FAIL);

                    if (NULL == (pRegTemplate = new FT_BYTE[iRecommendedRegFtrLen])) _com_issue_error(E_OUTOFMEMORY);

                    FT_BOOL bRegSucceeded = FT_FALSE;
                    rc = MC_generateRegFeatures(m_mcContext,              // Matching Context
                                                m_mcRegOptions,           // Options - collected from checkboxes in the dialog
                                                m_NumberOfPreRegFeatures, // Number of Pre-Enrollment feature sets submitted. It must be number received previously from the SDK.
                                                iRecommendedPreRegFtrLen, // Length of every Pre-Enrollment feature sets in the array
                                                m_TemplateArray,          // Array of pointers to the Pre-Enrollment feature sets
                                                iRecommendedRegFtrLen,    // Length of the Enrollment Features(template) buffer received previously from the SDK
                                                pRegTemplate,             // Pointer to the buffer where the Enrollment template to be stored
                                                NULL,                     // Reserved. Must always be NULL.
                                                &bRegSucceeded);          // Returned result of Enrollment Template creation. FT_TRUE means extracted OK.

                    if (FT_OK <= rc && bRegSucceeded == FT_TRUE) {
                        // Enrollment template is generated.
                        // Normally it should be saved in some database, etc.
                        // Here we just put it into a BLOB to use later for verification.
                        m_RegTemplate.pbData = pRegTemplate;
                        m_RegTemplate.cbData = iRecommendedRegFtrLen;

                        pRegTemplate = NULL;   // This prevents deleting at the end of the function

                        /* output status message*/
                        ui->textEdit->setTextColor(QColor(Qt::green));
                        addStatus("Enrollment Template generated successfully");

                        if(saveTemplate()) {        //save the fingerprint image to file .bmp
//                            ui->pushButtonNext->setEnabled(true);
                            emit templateGenerated(true);
                            hasReadyTemplate = true;
                            setRawDataBlob(pFingerprintImage, iFingerprintImageSize);
                        }

                    }
                    else {
                        QMessageBox::information(this, "Fingerprint Enrollment", "Creation of Enrollment Template Failed.",
                                                 QMessageBox::Close|QMessageBox::Escape);
                        ui->lineEditPrompt->setText("Scan your finger for enrollment.");
                        // Enrolment faled, cleanup data.
                        m_nRegFingerprint = 0;

                        for (int i=0; i<m_NumberOfPreRegFeatures; ++i)
                            if(m_TemplateArray[i]) delete [] m_TemplateArray[i], m_TemplateArray[i] = NULL; // Delete Pre-Enrollment feature sets stored in the array.
                    }
                }
                else {
                    // This is normal cource of events, before all 4 fingerprint images are captured.
                    ui->lineEditPrompt->setText("Scan same finger again.");
                }
            }
            else {
                QMessageBox::information(this, "Fingerprint Enrollment", "Creation of Pre-Enrollment feature set Failed.",
                                         QMessageBox::Close|QMessageBox::Escape);
                ui->lineEditPrompt->setText("Scan your finger for enrollment.");
            }
        }
        else {
            QMessageBox::information(this, "Fingerprint Enrollment", "An Enrollment Template has already been created.\n\nClose the dialog, then select \"Fingerprint Verification\" or \"Save Fingerprint Enrollment Template\".",
                                     QMessageBox::Close|QMessageBox::Escape);
        }
    }
    catch(_com_error E) {
        hr = E.Error();
    }
    catch(...) {
        hr = E_UNEXPECTED;
    }
    delete [] pPreRegTemplate;
    delete [] pRegTemplate;
}


// This function simply saves BLOB of the Enrollment template created previously in to
// user-specified file.
bool FEDialog::saveTemplate()
{
    if (m_RegTemplate.cbData == 0 || m_RegTemplate.pbData == NULL) {
        QMessageBox::information(this, "Save Fingerprint Template", "Before attempting to save a template, select \"Fingerprint Enrollment\" to create a Fingerprint Enrollment Template or select \"Read Fingerprint Enrollment Template\" to read a template.",
                                 QMessageBox::Close|QMessageBox::Escape);
        return false;
    }

    QString fileName = getFPTTempFilePath("temp.fpt");

    if(!saveFile(fileName))
        return 0;

    return true;
}

bool FEDialog::saveFile(const QString &fileName)
{
    if (!writeFile(fileName)) {
        QMessageBox::critical(this, "Fingerprint Enroll", tr("Saving canceled"), QMessageBox::Close);
        return false;
    }
    return true;
}

bool FEDialog::writeFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
        QMessageBox::warning(this, tr("Save Fingerprint Template"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(file.fileName())
                             .arg(file.errorString()));
        return false;
    }

    /* open datastream */
    QDataStream datastream(&file);
    /* disable cursor from clickable */
    QApplication::setOverrideCursor(Qt::WaitCursor);
    /* write out Fingerprint template to file *.fpt */
    datastream.writeRawData((const char *) (m_RegTemplate.pbData), m_RegTemplate.cbData);
    /* Release cursor */
    QApplication::restoreOverrideCursor();

    return true;
}


void FEDialog::getRegTemplate(QJsonObject* fpJsonObj) const {
    if (hasReadyTemplate/*m_RegTemplate.cbData && m_RegTemplate.pbData*/) { // only copy template if it is not empty

        QByteArray fpTemplate((const char*)m_RegTemplate.pbData, m_RegTemplate.cbData);


        fpJsonObj->insert("pbdata", QJsonValue::fromVariant(fpTemplate.toBase64(QByteArray::Base64Encoding)));
        fpJsonObj->insert("cbdata", QJsonValue::fromVariant(fpTemplate.size()));
//QByteArray::KeepTrailingEquals
        //NOTE: helps to display Enrolling Reg Template in base63
//        qDebug() << "Enrolling Reg Template in base63: " << fpTemplate.toBase64(QByteArray::Base64Encoding).data();
    }
}

void FEDialog::getRawRegTemplate(DATA_BLOB& dataBlob) const {
    if (hasReadyTemplate/*m_RegTemplate.cbData && m_RegTemplate.pbData*/) { // only copy template if it is not empty
        // Delete the old stuff that may be in the template.
//        delete [] dataBlob.pbData;
        dataBlob.pbData = {0};
        dataBlob.pbData = NULL;
        //NOTE: Checking for data blob
//        qDebug() << dataBlob.cbData;
        dataBlob.cbData = 0;

        // Copy the new template, but only if it has been created.
        dataBlob.pbData = new BYTE[m_RawRegTemplate.cbData];
        if (!dataBlob.pbData) _com_issue_error(E_OUTOFMEMORY);
        ::CopyMemory(dataBlob.pbData, m_RawRegTemplate.pbData, m_RawRegTemplate.cbData);
        dataBlob.cbData = m_RawRegTemplate.cbData;
    }
}

void FEDialog::setRawDataBlob(FT_IMAGE_PT fingerprintImage, int imageSize)
{
    delete [] m_RawRegTemplate.pbData;
    m_RawRegTemplate.pbData = NULL;
    m_RawRegTemplate.cbData = 0;

    // Copy the new template, but only if it has been created.
    m_RawRegTemplate.pbData = new BYTE[imageSize];
    if (!m_RawRegTemplate.pbData) _com_issue_error(E_OUTOFMEMORY);
    ::CopyMemory(m_RawRegTemplate.pbData, fingerprintImage, imageSize);
    m_RawRegTemplate.cbData = imageSize;
}

void FEDialog::initContext()
{
    HRESULT hr = S_OK;

    try {
        FT_RETCODE rc = FT_OK;

#ifdef QT_DEBUG
        qDebug("Creating FX_ontext");
#endif
        // Create Context for Feature Extraction
        if (FT_OK != (rc = FX_createContext(&m_fxContext))) {
            QMessageBox::critical(this, "Fingerprint Enrollment", "Cannot create Feature Extraction Context.",
                                  QMessageBox::Close|QMessageBox::Escape);
            throw -1;
        }
#ifdef QT_DEBUG
        qDebug("FX_context created");
#endif

        // Create Context for Matching
        if (FT_OK != (rc = MC_createContext(&m_mcContext))) {
            QMessageBox::critical(this, "Fingerprint Enrollment", "Cannot create Matching Context.",
                                   QMessageBox::Close|QMessageBox::Escape);
            throw -2;
        }

#ifdef QT_DEBUG
        qDebug("MC_createContext created");
#endif

        // Get number of Pre-Enrollment feature sets needed to create on Enrollment template
        // allocate array that keeps those Pre-Enrollment and set the first index to 0;
        MC_SETTINGS mcSettings = {0};
        if (FT_OK != (rc = MC_getSettings(&mcSettings))) {
            QMessageBox::critical(this, "Fingerprint Enrollment", "Cannot get number of Pre-Reg feature sets needed to create one Enrollment template.",
                                   QMessageBox::Close|QMessageBox::Escape);
            throw -3;
        }

        m_NumberOfPreRegFeatures = mcSettings.numPreRegFeatures;
        if (NULL == (m_TemplateArray = new FT_BYTE*[m_NumberOfPreRegFeatures]))
           throw -4;

        ::ZeroMemory(m_TemplateArray, sizeof(FT_BYTE**)*m_NumberOfPreRegFeatures);

        m_nRegFingerprint = 0;  // This is index of the array where the first template is put.

        // Start Enrollment.
        DP_ACQUISITION_PRIORITY ePriority = DP_PRIORITY_NORMAL; // Using Normal Priority, i.e. fingerprint will be sent to
                                                                // this process only if it has active window on the desktop.
        HRESULT re;
        HWND hWnd = (HWND)this->winId();
        if(S_OK != (re = DPFPCreateAcquisition(ePriority, GUID_NULL, DP_SAMPLE_TYPE_IMAGE, hWnd, WMUS_FP_NOTIFY, &m_hOperationEnroll)))
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

        if(S_OK != DPFPStartAcquisition(m_hOperationEnroll))
        {
            throw -7;
        }

        ui->lineEditPrompt->setText("Scan your finger for enrollment.");

    }
    catch(int e)
    {
        hr = e;
    }

    catch(...)
    {
        hr = E_UNEXPECTED;
    }

    if(hr != S_OK)
    {
        ui->lineEditPrompt->setText("Error happened");
        QMessageBox::critical(this, "Fingerprint Enrol", "Enrolling error", QMessageBox::Close|QMessageBox::Escape);
        this->close();
    }

#ifdef QT_DEBUG
        qDebug("Fully initialized");
#endif
}
