#ifndef GITPROCESSDIALOG_H
#define GITPROCESSDIALOG_H

#include <QDialog>

namespace Ui {
class GitProcessDialog;
}

class GitProcessDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GitProcessDialog(QWidget *parent = 0);
    ~GitProcessDialog();

    void fetchUpdates();

private:
    Ui::GitProcessDialog *ui;
};

#endif // GITPROCESSDIALOG_H
