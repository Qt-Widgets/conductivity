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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include <QTimer>
#include <NIDAQmx.h>

#include "configureRvsTdialog.h"
#include "configureIvsVdialog.h"


namespace Ui {
class MainWindow;
}

QT_FORWARD_DECLARE_CLASS(QFile)
QT_FORWARD_DECLARE_CLASS(Keithley236)
QT_FORWARD_DECLARE_CLASS(LakeShore330)
QT_FORWARD_DECLARE_CLASS(Plot2D)

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

protected:
  void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
  bool CheckInstruments();
  bool startDAQ();
  void stopDAQ();
  bool getNewSigmaMeasure();
  int  JunctionCheck();
  void initPlots();

private slots:
  void on_startRvsTButton_clicked();
  void on_startIvsVButton_clicked();
  void onTimeToCheckReachedT();
  void onTimerStabilizeT();
  void onTimeToReadT();
  void onTimeToGetNewMeasure();
  void onComplianceEvent();
  void onKeithleyReadyForTrigger();
  void onNewKeithleyReading(QDateTime dataTime, QString sDataRead);

private:
  Ui::MainWindow *ui;

private:
  QFile*        pOutputFile;
  Keithley236*  pKeithley;
  LakeShore330* pLakeShore;
  QDateTime     currentTime;
  QDateTime     waitingTStartTime;
  QDateTime     startReadingTTime;
  QDateTime     startMeasuringTime;
  QTimer        waitingTStartTimer;
  QTimer        stabilizingTimer;
  QTimer        readingTTimer;
  QTimer        measuringTimer;

  ConfigureRvsTDialog configureRvsTDialog;
  ConfigureIvsVDialog configureIvsVDialog;

  const quint8  LAMP_ON;
  const quint8  LAMP_OFF;
  int           iCurrentTPlot;
  int           gpibBoardID;
  int32         error;
  TaskHandle    lampTaskHandle;// Digital Output
  QString       sLampLine;
  quint8        currentLampStatus;
  bool          bStartDaq;
  Plot2D       *pPlotMeasurements;
  Plot2D       *pPlotTemperature;
  QString       sMeasurementPlotLabel;
  QString       sTemperaturePlotLabel;
  int           maxPlotPoints;
  quint64       maxReachingTTime;
  double        timeBetweenMeasurements;
  int           iPlotDark;
  int           iPlotPhoto;
  volatile bool isK236ReadyForTrigger;
  bool          bRunning;
};

#endif // MAINWINDOW_H
