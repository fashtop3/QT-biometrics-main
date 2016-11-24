#include "evdialog.h"
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
