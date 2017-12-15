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
#if !defined(Q_PROCESSOR_ARM)
#include <QSerialPort>
#endif

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

signals:
  void IForwardDone();
  void VReverseDone();

protected:
  void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
  bool CheckInstruments();
  bool getNewMeasure();
  void initRvsTPlots();
  void stopRvsT();
  void startI_V();
  void stopIvsV();
  void initIvsVPlots();
  bool prepareOutputFile(QString sBaseDir, QString sFileName);
#if defined(Q_PROCESSOR_ARM)
  bool initPWM();
#else
  bool connectToArduino();
  bool writeToArduino(QByteArray requestData);
#endif
  bool switchLampOn();
  bool switchLampOff();

private slots:
  void on_startRvsTButton_clicked();
  void on_startIvsVButton_clicked();
  void onTimeToCheckReachedT();
  void onTimeToCheckT();
  void onTimerStabilizeT();
  void onTimeToReadT();
  void onTimeToGetNewMeasure();
  void onComplianceEvent();
  void onKeithleyReadyForTrigger();
  void onNewKeithleyReading(QDateTime dataTime, QString sDataRead);
  bool onKeithleyReadyForSweepTrigger();
  void onKeithleySweepDone(QDateTime dataTime, QString sData);
  void onIForwardSweepDone(QDateTime, QString sData);
  void onVReverseSweepDone(QDateTime, QString sData);

private:
  Ui::MainWindow *ui;

  enum commands {
    ACK         = 6,
    NACK        = 21,
    EOS         = 127,
    AreYouThere = 70,
    SwitchON    = 71,
    SwitchOFF   = 72
  };
  commands command;
  enum measure {
      NoMeasure   = 0,
      RvsTSourceI = 1,
      RvsTSourceV = 2,
      IvsVSourceI = 3,
      IvsVSourceV = 4,
      IvsV        = 5
  };
  measure presentMeasure;

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

  double        setPointT;
  const quint8  LAMP_ON;
  const quint8  LAMP_OFF;
  int           iCurrentTPlot;
  int           gpibBoardID;
  quint8        currentLampStatus;
  Plot2D       *pPlotMeasurements;
  Plot2D       *pPlotTemperature;
  QString       sMeasurementPlotLabel;
  QString       sTemperaturePlotLabel;
  int           maxPlotPoints;
  quint64       maxReachingTTime;
  int           iPlotDark;
  int           iPlotPhoto;
  volatile bool isK236ReadyForTrigger;
  bool          bRunning;
  int           junctionDirection;

#if defined(Q_PROCESSOR_ARM)
  unsigned      lampPin;
  int           gpioHostHandle;
  unsigned      PWMfrequency;
  quint32       dutyCycle;
  double        pulseWidthAt0;
  double        pulseWidthAt180;
#else
  QSerialPort   serialPort;
  QByteArray    requestData;
  enum QSerialPort::BaudRate
                baudRate;
  int           waitTimeout;
#endif
};

#endif // MAINWINDOW_H
