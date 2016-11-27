#ifndef EVDIALOG_H
#define EVDIALOG_H

#include <QDialog>


namespace Ui {
class EVDialog;
}

class EVDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EVDialog(QWidget *parent = 0);
    ~EVDialog();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::EVDialog *ui;

};

#endif // EVDIALOG_H
