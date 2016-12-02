#include <QApplication>
#include "dpRCodes.h"
#include "dpDefs.h"
#include "DPDevClt.h"
#include "dpFtrEx.h"
#include "dpMatch.h"
#include "runguard.h"

#include <QMessageBox>
#include <evdialog.h>
#include <QDebug>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{

    RunGuard guard( "JFDBHJ87867SHBJKSD7687EKJ" );
        if ( !guard.tryToRun() )
            return 0;

    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("Biometric Capturing Application");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Student Biometric capturing application");
    parser.addHelpOption();
    parser.addVersionOption();

    // A boolean option with a single name (-l)
    QCommandLineOption xmlFileChangedOption("l", QCoreApplication::translate("main", "Option for capture.xml file changed event"));
    parser.addOption(xmlFileChangedOption);

    parser.process(app);

    bool xmlFileChanged = parser.isSet(xmlFileChangedOption);

     try
    {
        DPFPInit();                             //initialize the module

        ULONG device = 0;
        GUID *ppDevUID;
        if (S_OK == DPFPEnumerateDevices(&device, &ppDevUID))    //check device connected
        {
            if(device == 0){
                throw -1;
            }
        }

        if(FT_OK == FX_init())                  //initialize feature extraction
        {
            if(FT_OK == MC_init())              //intialize sdk matching
            {
                //call exec on evdialog
                EVDialog dialog;
                if(xmlFileChanged) {
                    dialog.onFileChanged();
                }
                else{
                    dialog.activateWindow();
                    dialog.exec();
                }

                MC_terminate();                 // All MC_init  must be matched with MC_terminate to free up the resources
            }
            else {
                QMessageBox::critical(0, "Device Error", "Cannot initialize Matching.",
                                      QMessageBox::Close | QMessageBox::Escape);
            }

            FX_terminate();             // All FX_init  must be matched with FX_terminate to free up the resources
        }
        else{
            QMessageBox::critical(0, "Device Error", "Cannot initialize Feature Extraction.",
                                  QMessageBox::Close | QMessageBox::Escape);
        }

        DPFPTerm();                     // All DPFPInit must be matched with DPFPTerm     to free up the resources
    }
    catch (int e)
    {
        qDebug("An exception occurred. Devive not found. ");
        QMessageBox::critical(0, "Device Error", "An exception occurred. Devive not found. ",
                              QMessageBox::Close | QMessageBox::Escape);
        return 0;
    }


    return 0;
}
