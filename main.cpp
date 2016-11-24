#include <QApplication>
#include "dpRCodes.h"
#include "dpDefs.h"
#include "DPDevClt.h"
#include "dpFtrEx.h"
#include "dpMatch.h"

#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QProcess>
#include <QDialog>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString init_message;

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


    return a.exec();
}
