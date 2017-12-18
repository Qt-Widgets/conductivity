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
    int    iWaitTime;
    int    iNSweepPoints;

    double dTStart;
    double dTStop;
    double dTStep;
    bool   bUseThermostat;
    int    iReachingTStart;
    int    iTimeToSteadyT;

    QString sSampleInfo;
    QString sBaseDir;
    QString sOutFileName;

protected:
    void initUI();
    void restoreSettings();
    void saveSettings();
    void setToolTips();
    void closeEvent(QCloseEvent *event);
    void setCaptions();
    bool isCurrentValid(double dCurrent);
    bool isVoltageValid(double dVoltage);
    bool isWaitTimeValid(int iWaitTime);
    bool isSweepPointNumberValid(int nSweepPoints);
    bool isTemperatureValid(double dTemperature);
    bool isTStepValid(double dTStep);
    bool isReachingTimeValid(int iReachingTime);
    bool isTimeToSteadyTValid(int iTime);

private slots:
    void on_outFilePathButton_clicked();
    void on_cancelButton_clicked();
    void on_doneButton_clicked();
    void on_ThermostatCheckBox_stateChanged(int arg1);
    void on_IStartEdit_textChanged(const QString &arg1);
    void on_IStopEdit_textChanged(const QString &arg1);
    void on_VStartEdit_textChanged(const QString &arg1);
    void on_VStopEdit_textChanged(const QString &arg1);
    void on_TStartEdit_textChanged(const QString &arg1);
    void on_TStopEdit_textChanged(const QString &arg1);
    void on_waitTimeEdit_textChanged(const QString &arg1);
    void on_sweepPointsEdit_textChanged(const QString &arg1);
    void on_MaxTimeToTStartEdit_textChanged(const QString &arg1);
    void on_TimeToSteadyTEdit_textChanged(const QString &arg1);

    void on_TStepEdit_textChanged(const QString &arg1);

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
    const int    waitTimeMin;
    const int    waitTimeMax;
    const int    nSweepPointsMin;
    const int    nSweepPointsMax;
    const int    reachingTMin;
    const int    reachingTMax;
    const int    timeToSteadyTMin;
    const int    timeToSteadyTMax;

    // Dialog user interace
    Ui::ConfigureIvsVDialog *ui;
};

#endif // CONFIGUREIVSVDIALOG_H
