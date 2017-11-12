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

public:
  QString sSampleInfo;
  QString sBaseDir;
  QString sOutFileName;

protected:
  void restoreSettings();
  void saveSettings();
  void setToolTips();
  void closeEvent(QCloseEvent *event);

private slots:
  void on_outFilePathButton_clicked();
  void on_cancelButton_clicked();
  void on_doneButton_clicked();

private:
  // QLineEdit styles
  QString     sNormalStyle;
  QString     sErrorStyle;
  // Dialog user interace
  Ui::ConfigureIvsVDialog *ui;
};

#endif // CONFIGUREIVSVDIALOG_H
