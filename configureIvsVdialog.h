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
  double dIStart;
  double dIStop;
  double dVStart;
  double dVStop;
  double dTemperature;
  bool   bUseThermostat;
  QString sSampleInfo;
  QString sBaseDir;
  QString sOutFileName;

protected:
  void restoreSettings();
  void saveSettings();
  void setToolTips();
  void closeEvent(QCloseEvent *event);
  void setCaptions();
  bool isCurrentValid(double dCurrent);
  bool isVoltageValid(double dVoltage);
  bool isTemperatureValid(double dTemperature);

private slots:
  void on_outFilePathButton_clicked();
  void on_cancelButton_clicked();
  void on_doneButton_clicked();
  void on_ThermostatCheckBox_stateChanged(int arg1);
  void on_IStartEdit_textChanged(const QString &arg1);
  void on_IStopEdit_textChanged(const QString &arg1);
  void on_VStartEdit_textChanged(const QString &arg1);
  void on_VStopEdit_textChanged(const QString &arg1);
  void on_TValueEdit_textChanged(const QString &arg1);

private:
  // QLineEdit styles
  QString     sNormalStyle;
  QString     sErrorStyle;
  // Limit Values
  const double currentMin;
  const double currentMax;
  const double voltageMin;
  const double voltageMax;
  const double temperatureMin;
  const double temperatureMax;
  // Dialog user interace
  Ui::ConfigureIvsVDialog *ui;
};

#endif // CONFIGUREIVSVDIALOG_H
