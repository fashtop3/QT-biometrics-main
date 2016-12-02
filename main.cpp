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
#include "xmlreadwrite.h"

int main(int argc, char *argv[])
{
//    XmlReadWrite xml;
//    xml.readXML();
//    return 0;

    RunGuard guard( "some_random_key" );
        if ( !guard.tryToRun() )
            return 0;

    QApplication a(argc, argv);

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
                dialog.exec();
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
