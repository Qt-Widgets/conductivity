#ifndef CONFIGUREIVSVDIALOG_H
#define CONFIGUREIVSVDIALOG_H

#include <QDialog>

namespace Ui {
class ConfigureIvsVDialog;
}

class ConfigureIvsVDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ConfigureIvsVDialog(QWidget *parent = 0);
  ~ConfigureIvsVDialog();

private slots:
  void on_cancelButton_clicked();
  void on_doneButton_clicked();

private:
  Ui::ConfigureIvsVDialog *ui;
};

#endif // CONFIGUREIVSVDIALOG_H
