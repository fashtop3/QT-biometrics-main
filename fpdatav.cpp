#include "fpdatav.h"
#include <stdio.h>
#include <QtWin>
#include <comdef.h>

#include "dpFtrEx.h"
#include "dpMatch.h"

#include <QThread>
#include <QDebug>
#include <QFileInfoList>
#include <QDir>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "fptpath.h"

//const GUID GUID_NULL = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };

FPDataV::FPDataV(QObject *parent) :
    QObject(parent),
    m_bDoLearning(FT_FALSE),
    m_hOperationVerify(0),
    m_fxContext(0),
    m_mcContext(0),
    matchFound(false)
{
    ::ZeroMemory(&m_RegTemplate, sizeof(m_RegTemplate));
    deviceInit();
}

void FPDataV::closeInit()
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
}

FPDataV::~FPDataV()
{
    closeInit();

    m_RegTemplate.pbData = {0};
    m_RegTemplate.cbData = 0;
    m_RegTemplate.pbData = NULL;

}

void FPDataV::loadRegTemplate(const DATA_BLOB& rRegTemplate) {
    // Delete the old stuff that may be in the template.
    delete [] m_RegTemplate.pbData;
    m_RegTemplate.pbData = NULL;
    m_RegTemplate.cbData = 0;

    // Copy Enrollment template data into member of this class
    m_RegTemplate.pbData = new BYTE[rRegTemplate.cbData];
    if (!m_RegTemplate.pbData) _com_issue_error(E_OUTOFMEMORY);
    ::CopyMemory(m_RegTemplate.pbData, rRegTemplate.pbData, rRegTemplate.cbData);
    m_RegTemplate.cbData = rRegTemplate.cbData;
}

void FPDataV::verify(FT_IMAGE_PT pFingerprintImage, int iFingerprintImageSize) {
    HRESULT hr = S_OK;
    matchFound = false;
    FT_BYTE* pVerTemplate = NULL;

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
                    qDebug() << buffer;
                }

                if (bVerified == FT_TRUE) {
                    qDebug() << "Fingerprint Matches!";
                    char buffer[101] = {0};
                    ULONG uSize = 100;
                    snprintf(buffer, uSize, "%lf", dFalseAcceptProbability);
                    qDebug() << buffer;
                    matchFound = true;
                    qDebug("Fingerprint Matches!");
                }
                else {
                    qDebug() << "Fingerprint did not Match!";
                    char buffer[101] = {0};
                    ULONG uSize = 100;
                    snprintf(buffer, uSize, "%lf", dFalseAcceptProbability);
                    qDebug() << buffer;
                    matchFound = false;
                    qDebug("Fingerprint did not Match!");
                }
            }
            else {
                char buffer[101] = {0};
                ULONG uSize = 100;
                snprintf(buffer, uSize, "Verification Operation failed, Error: %ld.", rc);
                qDebug() << buffer;
#ifdef QT_DEBUG
                qDebug() << "Scan your finger for verification again.";
#endif
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

void FPDataV::setFPData(const DATA_BLOB &data)
{
    dataCopy = data;
}

int FPDataV::verifyAll(const DATA_BLOB& dataBlob)
{
    matchFound = false;

    //TODO: move this to a new network class
    QNetworkRequest request(QUrl("http://localhost:8000/data"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, /*"application/x-www-form-urlencoded"*/ "application/json");

    QNetworkAccessManager man;
    QNetworkReply *reply = man.get(request);// post(request, QJsonDocument(json).toJson());

    while(!reply->isFinished())
    {
        qApp->processEvents();
    }


    QByteArray response_data = reply->readAll();
    QJsonDocument json_resp = QJsonDocument::fromJson(response_data);
    QJsonArray jsonArray = json_resp.array();

    int run = 0;
    foreach (const QJsonValue & value, jsonArray) {
        QJsonObject obj = value.toObject();

        QByteArray pdata = QByteArray::fromBase64(QByteArray("APh/Acgq43NcwEE3CatxsIAUVZIuT9wYRQ5kV3dl4ZUNMYA/t3cJdBu9DuDEO9aoDBsXI3Xh4q5nM9O0y3nEd0cCKL2tQdUTddPqv2lemLAVc5mwYz//AcsyYqNiFnNIUAxcp4A1aBYtq7UTtqp1d+GNvQ3hmPrcrhe2gfXL7JppjzPh8Ark/xvVmmGVX7JqTlmSyIBgbclYgc7KDTguW0WeL9Sh8mD0UMLqfPPiqXQgevCJ/lO2J1uo0IpZ9N1tAneCLYlKzAYm3LJ3D1ZGzPkG3MdnMzIct51hU4cJ9hPp+CVkacdd5KRSx0Gn1pj2XpuVqnoPDF7EMAIFkWqMOTQhTZvoVcG8NrNdOn5Uc6q1cIP//sabtXWZdJxrog4eJvDkKGKfBSspKY5OhIZlTcIs6QFA2XpXbXWdYo/P+Gg+UrDslHAwMERBt+cRIQ0s4JDsG1QbN2pM0SA1+aiIc42Ai0sWW4BaDf2SKNR89Q58cXjBvtGegGfza7lN/n9sMRz5bwD4BgHIKuNzXMBBNwmrcXCCFFWSwdj6pjtMYc3YCEtnJUYWX46XX/Pw78tyWor/mp7h6Ar2O4q6J3lhlSJHhKy+Z66gZ5+oMymWrIADlCi4ii00EoPgcQbjw28Qwzo8cShrvY30DeXfy8gKLBi5UGle3ADgnXiw8EqOg0CAZCiE508NOmbJ1DOUdddZlIPaozwbXR/PKld88P4v+7RNqVRXrl7dQXwqYFuhl8Z7fJbRE2lLFCkZ8jqdUYhmJ47j/TKIqjIczYeES6FPOHcoLdQ7z1AQS98ZJlLttWKml/kkwtyN8DwTu/nl9jdmtOMkxF5YUJ/LgPEsymQtutMMFigwEkjAX3jJocqsbwD4JwHIKuNzXMBBNwmrcbCbFFWSfqNLuCilo3lzGf7XrfkhxAo4nVnd8maj+cwewuTpKm9VDgf0OMOGo5i46DtdiIwpIgzRB3dpy71D0GMN0YwsSDhSQ4gCcanBB54r6am9soD0tyaFDgDGEmcMjj2pU2JcPtVFsf2ZVwD6/CcNn9QvRQf0T/tG14Gu/xsZHIT8NpW1EPEktpDMV7IYgqtHzisRFem5irjQnXhsDNSwT6l7qjaE0cfpfXxOk9bEYMoXnEz5jsX7oLhje6BFy0ok+p8kW2vmNYkybtcdLlaouRubvBrYRsQVVWIxbkVj1S/vHvc9ouN3jGPM8i/yU3AwXXXpD3hWGaaYQigecP67tCc7hEYtXIyiICEvbMWjkQmyBT3oopq8iYkGbwDoKQHIKuNzXMBBNwmrcfCPFFWS1FDt52GQkCU16o4PDbim/t+HOK/KCSIHfew/TwLBsCBI7PAmqA8fHR4U75AXrSBAHTMYNOM4CwEH50jVO/CAHO7MvqGPbAr+U7C04Y34TO+TXyOLziRlw8EznWsSrFqlfJ2jTR98FrJJP3jstjrAfoAmwF4d/+YfY5rPm8KCz63AO/5lARmPYkc5msWhFc10wiI/mtIScppVca1F4RyzBHL1EKI4v3AtUVS7YkPXDINvPdMs6JOh619N9sGCHhVlSAtX6Ll6ZCoxzGjIqiUVTlrOxdOhI8kP6IE5DgNr/cpzZgiXP0S9xYRrJd++5dzg1FunUW5v1xSJZKyoxr5km7SsaSr1WPquasoaHt4YIk8jmGzj5pHvDrJvAAAAQAEAAIABAABAAQAAQAEAAIABAABAAQAAQAEAAAABAACAAQAAgAAAAIAAAACAAQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA")/*obj["pbdata"].toVariant().toByteArray()*/);

        qDebug() /*<< obj["pbdata"].toVariant().toByteArray().data()*/ << "from base size: " << pdata.size() ;
        /**
         * calling verification method with the features gotten from Enrolling Diaoolog
         * and implicitly matches it with all the .fpt s'
         */
        verify((BYTE *) pdata.data(), pdata.size());
        if(matchFound)
        {
            qDebug() << "match found";
            break; //if match is found break the loop and return the match status
        }else{
            qDebug() << "no match found from test";
        }

        ++run;
        emit progress(run, jsonArray.count());
    }

    reply->deleteLater();

    return matchFound;
}

void FPDataV::onStartVerification(const DATA_BLOB &blob)
{
    bool isMatched = verifyAll(blob);

    emit done(isMatched);
    onTimeout();
}

void FPDataV::onTimeout()
{
    qDebug() << "from the verify thread" << QThread::currentThreadId();
}


void FPDataV::deviceInit()
{
    HRESULT hr = S_OK;
    try {
        FT_RETCODE rc = FT_OK;

        // Create Context for Feature Extraction
        if (FT_OK != (rc = FX_createContext(&m_fxContext))) {
            return;  // return TRUE  unless you set the focus to a control
        }

        // Create Context for Matching
        if (FT_OK != (rc = MC_createContext(&m_mcContext))) {
            return;  // return TRUE  unless you set the focus to a control
        }


        // Start Verification.
        DP_ACQUISITION_PRIORITY ePriority = DP_PRIORITY_NORMAL; // Using Normal Priority, i.e. fingerprint will be sent to
                                              // this process only if it has active window on the desktop.
        HRESULT re;
        HWND m_hWnd = 0;//(HWND)this->winId();
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

    }
    catch(_com_error& E) {
        hr = E.Error();
    }
    catch(...) {
        hr = E_UNEXPECTED;
    }

    if (FAILED(hr)) {
    }
}
