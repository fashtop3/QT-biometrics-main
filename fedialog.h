#ifndef FEDIALOG_H
#define FEDIALOG_H

#include <QDialog>

namespace Ui {
class FEDialog;
}

class FEDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FEDialog(QWidget *parent = 0);
    ~FEDialog();

private:
    Ui::FEDialog *ui;
};

#endif // FEDIALOG_H
