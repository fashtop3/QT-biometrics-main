#include "fedialog.h"
#include "ui_fedialog.h"

FEDialog::FEDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FEDialog)
{
    ui->setupUi(this);
}

FEDialog::~FEDialog()
{
    delete ui;
}
