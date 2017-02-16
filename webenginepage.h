#ifndef WEBENGINEPAGE_H
#define WEBENGINEPAGE_H

#include <QWebEnginePage>

class WebEnginePage : public QWebEnginePage
{
public:
    explicit WebEnginePage(QWidget *parent = 0);
    virtual ~WebEnginePage();

    bool certificateError(const QWebEngineCertificateError &certificateError);
};

#endif // WEBENGINEPAGE_H
