#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QJsonObject>
#include <QThread>

#include "fedialog.h"
#include "fpdatav.h"
#include "webview.h"
#include "logindialog.h"



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void startVerification(const DATA_BLOB &blob);
    void doneCapturing(const QString cid, int statusCode, const QString statusText, QString &responseData = QString());

private slots:
    void onInitCapturing(const QString cid);

protected:
    void startEnrollment();

private:
    Ui::MainWindow *ui;
    WebView* webView;
    LoginDialog *login;

    DATA_BLOB  m_RegTemplate;   // BLOB that keeps Enrollment Template. It is used to pass it from Enrollment to Verification and also for saving/reading from file.
    DATA_BLOB  raw_RegTemplate;
    QThread workerThread;

    FEDialog *feDialog;
    FPDataV *fpWorker;
    QJsonObject *fpJsonObject;

};

#endif // MAINWINDOW_H
