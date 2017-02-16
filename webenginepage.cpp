#include "webenginepage.h"
#include <QUrl>

WebEnginePage::WebEnginePage(QWidget *parent) :
    QWebEnginePage(parent)
{

}

WebEnginePage::~WebEnginePage()
{

}

bool WebEnginePage::certificateError(const QWebEngineCertificateError &certificateError)
{
    return true;
}
