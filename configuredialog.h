#ifndef CONFIGUREDIALOG_H
#define CONFIGUREDIALOG_H

#include <QDialog>

QT_FORWARD_DECLARE_CLASS(QDoubleValidator)

namespace Ui {
class ConfigureDialog;
}

class ConfigureDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ConfigureDialog(QWidget *parent = 0);
  ~ConfigureDialog();

public:
  bool   bSourceI;
  double dSourceValue;
  double dTempStart;
  double dTempEnd;
  int    iSweepTime;

protected:
  void closeEvent(QCloseEvent *event);
  void setCaptions(bool bSourceI);
  bool isSourceValueValid();
  bool isTemperatureValueValid(double dTemperature);

private slots:
  void on_radioButtonSourceI_clicked();
  void on_radioButtonSourceV_clicked();

  void on_testValueEdit_textChanged(const QString &arg1);

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
  const int    sweepTimeMin;
  const int    sweepTimeMax;
  // Dialog user interace
  Ui::ConfigureDialog *ui;
};

#endif // CONFIGUREDIALOG_H
