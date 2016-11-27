#include "evdialog.h"
#include "fedialog.h"
#include "fvdialog.h"
#include "ui_evdialog.h"




EVDialog::EVDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EVDialog)
{
    ui->setupUi(this);


}

EVDialog::~EVDialog()
{
    delete ui;
}

void EVDialog::on_pushButton_clicked()
{
    FEDialog dialog;
    dialog.exec();
}

void EVDialog::on_pushButton_2_clicked()
{
    FVDialog dialog;
    dialog.exec();
}
