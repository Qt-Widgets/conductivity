/*
 *
Copyright (C) 2016  Gabriele Salvato

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/
#ifndef ConfigureRvsTDialog_H
#define ConfigureRvsTDialog_H

#include <QDialog>

QT_FORWARD_DECLARE_CLASS(QDoubleValidator)

namespace Ui {
class ConfigureRvsTDialog;
}

class ConfigureRvsTDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ConfigureRvsTDialog(QWidget *parent = 0);
  ~ConfigureRvsTDialog();

public:
  bool   bSourceI;
  double dSourceValue;
  double dTempStart;
  double dTempEnd;
  double dTRate;
  QString sSampleInfo;
  QString sBaseDir;
  QString sOutFileName;

protected:
  void restoreSettings();
  void saveSettings();
  void setToolTips();
  void closeEvent(QCloseEvent *event);
  void setCaptions(bool bSourceI);
  bool isSourceValueValid();
  bool isTemperatureValueValid(double dTemperature);
  bool isTRateValid(double dTRate);

private slots:
  void on_radioButtonSourceI_clicked();
  void on_radioButtonSourceV_clicked();
  void on_testValueEdit_textChanged(const QString &arg1);
  void on_TStartEdit_textChanged(const QString &arg1);
  void on_TEndEdit_textChanged(const QString &arg1);
  void on_TRateEdit_textChanged(const QString &arg1);
  void on_doneButton_clicked();
  void on_outFilePathButton_clicked();
  void on_outFileEdit_editingFinished();


  void on_cancelButton_clicked();

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
  const double TRateMin;
  const double TRateMax;
  // Dialog user interace
  Ui::ConfigureRvsTDialog *ui;
};

#endif // ConfigureRvsTDialog_H
