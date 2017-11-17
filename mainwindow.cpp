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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "utility.h"
#include "keithley236.h"
#include "lakeshore330.h"
#include "plot2d.h"

#include <QMessageBox>
#include <QDebug>
#include <QSettings>
#include <QFile>
#include <QThread>
#include <ni4882.h>
#include <NIDAQmx.h>


#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error;


MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , pOutputFile(Q_NULLPTR)
  , pKeithley(Q_NULLPTR)
  , pLakeShore(Q_NULLPTR)
  , LAMP_ON(1)
  , LAMP_OFF(0)
  , gpibBoardID(0)
  , lampTaskHandle(0)
  , sLampLine(QString("NiUSB-6211/port1/line0"))
  , currentLampStatus(LAMP_OFF)
  , pPlotMeasurements(Q_NULLPTR)
  , pPlotTemperature(Q_NULLPTR)
  , maxPlotPoints(3000)
  , maxReachingTTime(120)// In seconds
  , timeBetweenMeasurements(5000)
  , iPlotDark(1)
  , iPlotPhoto(2)
  , isK236ReadyForTrigger(false)
  , bRunning(false)
{
  ui->setupUi(this);

  ui->startRvsTButton->show();
  ui->startIvsVButton->show();

  QSettings settings;
  restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
  restoreState(settings.value("mainWindowState").toByteArray());
}


MainWindow::~MainWindow() {
  if(pKeithley != Q_NULLPTR) delete pKeithley;
  pKeithley = Q_NULLPTR;
  if(pLakeShore != Q_NULLPTR) delete pLakeShore;
  pLakeShore = Q_NULLPTR;

  if(pPlotMeasurements) delete pPlotMeasurements;
  pPlotMeasurements = Q_NULLPTR;
  if(pPlotTemperature) delete pPlotTemperature;
  pPlotTemperature = Q_NULLPTR;

  delete ui;
}


void
MainWindow::closeEvent(QCloseEvent *event) {
  Q_UNUSED(event)
  QSettings settings;
  settings.setValue("mainWindowGeometry", saveGeometry());
  settings.setValue("mainWindowState", saveState());
  if(bRunning) {
    waitingTStartTimer.stop();
    stabilizingTimer.stop();
    readingTTimer.stop();
    measuringTimer.stop();
    disconnect(&waitingTStartTimer, 0, 0, 0);
    disconnect(&stabilizingTimer, 0, 0, 0);
    disconnect(&readingTTimer, 0, 0, 0);
    disconnect(&measuringTimer, 0, 0, 0);
    if(pOutputFile) {
      if(pOutputFile->isOpen())
        pOutputFile->close();
      pOutputFile->deleteLater();
      pOutputFile = Q_NULLPTR;
    }
    if(pKeithley) pKeithley->endVvsT();
    stopDAQ();
    if(pLakeShore) pLakeShore->switchPowerOff();
  }
}


bool
MainWindow::CheckInstruments() {
  Addr4882_t padlist[31];
  Addr4882_t resultlist[31];
  for(short i=0; i<30; i++) padlist[i] = i+1;
  padlist[30] = NOADDR;

  SendIFC(gpibBoardID);
  if(ThreadIbsta() & ERR) {
    qDebug() << "In SendIFC: ";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return false;
  }

  // If addrlist contains only the constant NOADDR,
  // the Universal Device Clear (DCL) message is sent
  // to all the devices on the bus
  Addr4882_t addrlist;
  addrlist = NOADDR;
  DevClearList(gpibBoardID, &addrlist);
  if(isGpibError("MainWindow::CheckInstruments() - DevClearList() failed. Are the Instruments Connected and Switced On ?"))
    return false;

  FindLstn(gpibBoardID, padlist, resultlist, 30);
  if(isGpibError("MainWindow::CheckInstruments() - FindLstn() failed. Are the Instruments Connected and Switced On ?"))
    return false;
  int nDevices = ThreadIbcntl();

  // Identify the instruments connected to the GPIB Bus
  char readBuf[257];
  QString sCommand = "*IDN?\r\n";
  for(int i=0; i<nDevices; i++) {
    Send(gpibBoardID, resultlist[i], sCommand.toUtf8().constData(), sCommand.length(), DABend);
    if(isGpibError("MainWindow::CheckInstruments() - *IDN? Failed"))
      return false;
    Receive(gpibBoardID, resultlist[i], readBuf, 256, STOPend);
    if(isGpibError("MainWindow::CheckInstruments() - Receive() Failed"))
      return false;
    QString sInstrumentID = QString(readBuf);
    // La source Measure Unit K236 non risponde al comando di identificazione !!!!
    if(sInstrumentID.contains("NSDCI", Qt::CaseInsensitive)) {
      if(pKeithley == Q_NULLPTR) {
        pKeithley = new Keithley236(gpibBoardID, resultlist[i], this);
        isK236ReadyForTrigger = false;
        connect(pKeithley, SIGNAL(complianceEvent()),
                this, SLOT(onComplianceEvent()));
        connect(pKeithley, SIGNAL(readyForTrigger()),
                this, SLOT(onKeithleyReadyForTrigger()));
        connect(pKeithley, SIGNAL(newReading(QDateTime, QString)),
                this, SLOT(onNewKeithleyReading(QDateTime, QString)));
      }
    } else if(sInstrumentID.contains("MODEL330", Qt::CaseInsensitive)) {
      if(pLakeShore == NULL)
        pLakeShore = new LakeShore330(gpibBoardID, resultlist[i], this);
    }
  }

  if(pKeithley == Q_NULLPTR) {
    int iAnswer = QMessageBox::warning(this,
                                       "Warning",
                                       "Source Measure Unit not Connected",
                                       QMessageBox::Abort|QMessageBox::Ignore,
                                       QMessageBox::Abort);
    if(iAnswer == QMessageBox::Abort)
      return false;
  }

  if(pLakeShore == Q_NULLPTR) {
    int iAnswer = QMessageBox::warning(this,
                                       "Warning",
                                       "Lake Shore not Connected",
                                       QMessageBox::Abort|QMessageBox::Ignore,
                                       QMessageBox::Abort);
    if(iAnswer == QMessageBox::Abort)
      return false;
  }
  return true;
}


bool
MainWindow::startDAQ() {
  // DAQmx Configure Digital Output Code
  DAQmxErrChk (
    DAQmxCreateTask("Switch the Lamp", &lampTaskHandle)
  );
  DAQmxErrChk (
      DAQmxCreateDOChan(
        lampTaskHandle,
        sLampLine.toLatin1().constData(),
        "Lamp Switch",
        DAQmx_Val_ChanPerLine)
  );
  DAQmxSetDOOutputDriveType(
    lampTaskHandle,
    sLampLine.toLatin1().constData(),
    DAQmx_Val_ActiveDrive);
  // DAQmx Start Code
  DAQmxErrChk (
    DAQmxStartTask(lampTaskHandle)
  );
  // Write Initial Lamp Status
  DAQmxErrChk(
    DAQmxWriteDigitalLines(
      lampTaskHandle,
      1,                       // numSampsPerChan
      1,                       // autoStart
      1.0,                     // timeout [s]
      DAQmx_Val_GroupByChannel,// dataLayout
      &currentLampStatus,      // writeArray[]
      NULL,                    // *sampsPerChanWritten
      NULL)                    // *reserved
  );
  return true;

Error:
  if(DAQmxFailed(error)) {
    char errBuf[2048];
    DAQmxGetExtendedErrorInfo(errBuf, sizeof(errBuf));
    DAQmxClearTask(lampTaskHandle);
    QMessageBox::critical(Q_NULLPTR, "DAQmx Error", QString(errBuf));
  }
  return false;
}


void
MainWindow::stopDAQ() {
  if(lampTaskHandle) {
    currentLampStatus  = LAMP_OFF;
    DAQmxWriteDigitalLines(
      lampTaskHandle,
      1,                       // numSampsPerChan
      1,                       // autoStart
      1.0,                     // timeout [s]
      DAQmx_Val_GroupByChannel,// dataLayout
      &currentLampStatus,      // writeArray[]
      NULL,                    // *sampsPerChanWritten
      NULL);                   // *reserved
    DAQmxStopTask(lampTaskHandle);
    DAQmxClearTask(lampTaskHandle);
  }
  lampTaskHandle = Q_NULLPTR;
}


void
MainWindow::on_startRvsTButton_clicked() {
  if(ui->startRvsTButton->text().contains("Stop")) {
    waitingTStartTimer.stop();
    stabilizingTimer.stop();
    readingTTimer.stop();
    measuringTimer.stop();
    disconnect(&waitingTStartTimer, 0, 0, 0);
    disconnect(&stabilizingTimer, 0, 0, 0);
    disconnect(&readingTTimer, 0, 0, 0);
    disconnect(&measuringTimer, 0, 0, 0);
    if(pOutputFile) {
      if(pOutputFile->isOpen())
        pOutputFile->close();
      pOutputFile->deleteLater();
      pOutputFile = Q_NULLPTR;
    }
    if(pKeithley) pKeithley->endVvsT();
    stopDAQ();
    if(pLakeShore) pLakeShore->switchPowerOff();
    ui->startRvsTButton->setText("Start R vs T");
    ui->startIvsVButton->setEnabled(true);
    return;
  }
  // else
  if(configureRvsTDialog.exec() == QDialog::Rejected)
    return;

  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  // Start the Digital Output Tasks
  ui->statusBar->showMessage("Checking for the Presence of Lamp Switch");
  if(!startDAQ()) {
    ui->statusBar->showMessage("National Instruments DAQ Board not present");
    QApplication::restoreOverrideCursor();
    return;
  }
  // Are the instruments connectd and ready to start ?
  ui->statusBar->showMessage("Checking for the GPIB Instruments");
  if(!CheckInstruments()) {
    ui->statusBar->showMessage("GPIB Instruments not found");
    stopDAQ();
    QApplication::restoreOverrideCursor();
    return;
  }
  if(pKeithley  != Q_NULLPTR)  {
    ui->statusBar->showMessage("Initializing Keithley 236...");
    if(pKeithley->init()) {
      ui->statusBar->showMessage("Unable to Initialize Keithley 236...");
      stopDAQ();
      QApplication::restoreOverrideCursor();
      return;
    }
  }
  if(pLakeShore != Q_NULLPTR) {
    ui->statusBar->showMessage("Initializing LakeShore 330...");
    if(pLakeShore->init()) {
      ui->statusBar->showMessage("Unable to Initialize LakeShore 330...");
      stopDAQ();
      QApplication::restoreOverrideCursor();
      return;
    }
  }

  // Open the Output file
  ui->statusBar->showMessage("Opening Output file...");
  if(pOutputFile) {
    if(pOutputFile->isOpen())
      pOutputFile->close();
    pOutputFile->deleteLater();
    pOutputFile = Q_NULLPTR;
  }
  pOutputFile = new QFile(configureRvsTDialog.sBaseDir + "/" + configureRvsTDialog.sOutFileName);
  if(!pOutputFile->open(QIODevice::Text|QIODevice::WriteOnly)) {
    QMessageBox::critical(this,
                          "Error: Unable to Open Output File",
                          QString("%1/%2")
                          .arg(configureRvsTDialog.sBaseDir)
                          .arg(configureRvsTDialog.sOutFileName));
    ui->statusBar->showMessage("Unable to Open Output file...");
    QApplication::restoreOverrideCursor();
    return;
  }

  initPlots();

  // Configure Source-Measure Unit
  double dAppliedCurrent = configureRvsTDialog.dSourceValue;
  double dVoltageCompliance = 10.0;
  pKeithley->initVvsT(dAppliedCurrent, dVoltageCompliance);

  // Configure Thermostat
  pLakeShore->setTemperature(configureRvsTDialog.dTempStart);
  pLakeShore->switchPowerOn(3);

  connect(&waitingTStartTimer, SIGNAL(timeout()),
          this, SLOT(onTimeToCheckReachedT()));
  connect(&readingTTimer, SIGNAL(timeout()),
          this, SLOT(onTimeToReadT()));
  waitingTStartTime = QDateTime::currentDateTime();

  // Read and plot initial value of Temperature
  startReadingTTime = waitingTStartTime;
  onTimeToReadT();
  readingTTimer.start(5000);

  // Start Reaching the Initial Temperature
  waitingTStartTimer.start(5000);

  ui->startIvsVButton->setDisabled(true);
  ui->startRvsTButton->setText("Stop R vs T");
  ui->statusBar->showMessage(QString("%1 Waiting Initial T[%2K]")
                             .arg(waitingTStartTime.toString())
                             .arg(configureRvsTDialog.dTempStart));
  QApplication::restoreOverrideCursor();
}


void
MainWindow::initPlots() {
  // Plot of Condicibility vs Temperature
  if(pPlotMeasurements) delete pPlotMeasurements;
  sMeasurementPlotLabel = QString("R^-1 [OHM^-1] vs T [K]");
  pPlotMeasurements = new Plot2D(this, sMeasurementPlotLabel);
  pPlotMeasurements->setMaxPoints(maxPlotPoints);
  pPlotMeasurements->SetLimits(0.0, 1.0, 0.0, 1.0, true, true, false, false);

  pPlotMeasurements->NewDataSet(iPlotDark,//Id
                                3, //Pen Width
                                QColor(127, 127, 127),// Color
                                pPlotMeasurements->ipoint,// Symbol
                                "Dark"// Title
                                );
  pPlotMeasurements->SetShowDataSet(iPlotDark, true);
  pPlotMeasurements->SetShowTitle(iPlotDark, true);

  pPlotMeasurements->NewDataSet(iPlotPhoto,//Id
                                3, //Pen Width
                                QColor(255, 255, 0),// Color
                                pPlotMeasurements->ipoint,// Symbol
                                "Photo"// Title
                                );
  pPlotMeasurements->SetShowDataSet(iPlotPhoto, true);
  pPlotMeasurements->SetShowTitle(iPlotPhoto, true);

  pPlotMeasurements->UpdatePlot();
  pPlotMeasurements->show();

  // Plot of Temperature vs Time
  if(pPlotTemperature) delete pPlotTemperature;
  sTemperaturePlotLabel = QString("T [K] vs t [s]");
  pPlotTemperature = new Plot2D(this, sTemperaturePlotLabel);
  pPlotTemperature->setMaxPoints(maxPlotPoints);
  pPlotTemperature->SetLimits(0.0, 1.0, 0.0, 1.0, true, true, false, false);

  pPlotTemperature->NewDataSet(1,//Id
                               3, //Pen Width
                               QColor(255, 0, 0),// Color
                               pPlotTemperature->ipoint,// Symbol
                               "T"// Title
                               );
  pPlotTemperature->SetShowDataSet(1, true);
  pPlotTemperature->SetShowTitle(1, true);

  pPlotTemperature->UpdatePlot();
  pPlotTemperature->show();
  iCurrentTPlot = 1;
}

void
MainWindow::onTimeToCheckReachedT() {
  double T = pLakeShore->getTemperature();
  if(fabs(T-configureRvsTDialog.dTempStart) < 0.1)//configureRvsTDialog->dTolerance)
  {
    disconnect(&waitingTStartTimer, 0, 0, 0);
    waitingTStartTimer.stop();

    connect(&stabilizingTimer, SIGNAL(timeout()),
            this, SLOT(onTimerStabilizeT()));
    stabilizingTimer.start(configureRvsTDialog.iStabilizingTime*60*1000);
    qDebug() << QString("Starting T Reached: Thermal Stabilization...");
    ui->statusBar->showMessage(QString("Starting T Reached: Thermal Stabilization for %1 min.").arg(configureRvsTDialog.iStabilizingTime));
  }
  else {
    currentTime = QDateTime::currentDateTime();
    quint64 elapsedMsec = waitingTStartTime.secsTo(currentTime);
//    qDebug() << "Elapsed:" << elapsedMsec << "Maximum:" << quint64(configureRvsTDialog.iReachingTime)*60*1000;
    if(elapsedMsec > quint64(configureRvsTDialog.iReachingTime)*60*1000) {
      disconnect(&waitingTStartTimer, 0, 0, 0);
      waitingTStartTimer.stop();
      connect(&stabilizingTimer, SIGNAL(timeout()),
              this, SLOT(onTimerStabilizeT()));
      stabilizingTimer.start(configureRvsTDialog.iStabilizingTime*60*1000);
      qDebug() << QString("Max Reaching Time Exceed...Thermal Stabilization...");
      ui->statusBar->showMessage(QString("Max Reaching Time Exceed...Thermal Stabilization for %1 min.").arg(configureRvsTDialog.iStabilizingTime));
    }
  }
}


void
MainWindow::onTimerStabilizeT() {
  // It's time to start measurements
  stabilizingTimer.stop();
  disconnect(&stabilizingTimer, 0, 0, 0);
  startMeasuringTime = QDateTime::currentDateTime();
  pPlotTemperature->NewDataSet(2,//Id
                               3, //Pen Width
                               QColor(255, 255, 0),// Color
                               StripChart::ipoint,// Symbol
                               "Tm"// Title
                               );
  pPlotTemperature->SetShowDataSet(2, true);
  pPlotTemperature->SetShowTitle(2, true);
  pPlotTemperature->UpdatePlot();
  iCurrentTPlot = 2;
  qDebug() << "Thermal Stabilization Reached: Start of the Measure";
  ui->statusBar->showMessage(QString("Thermal Stabilization Reached: Start of the Measure"));
  connect(&measuringTimer, SIGNAL(timeout()),
          this, SLOT(onTimeToGetNewMeasure()));
  if(!pLakeShore->startRamp(configureRvsTDialog.dTempEnd, configureRvsTDialog.dTRate)) {
    ui->statusBar->showMessage(QString("Error Starting the Measure"));
    return;
  }
  measuringTimer.start(timeBetweenMeasurements);
  bRunning = true;
}


void
MainWindow::onTimeToGetNewMeasure() {
  getNewSigmaMeasure();
  if(!pLakeShore->isRamping()) {// Ramp is Done
    measuringTimer.stop();
    disconnect(&measuringTimer, 0, 0, 0);
    if(pOutputFile) {
      if(pOutputFile->isOpen())
        pOutputFile->close();
      pOutputFile->deleteLater();
      pOutputFile = Q_NULLPTR;
    }
    pKeithley->endVvsT();
    stopDAQ();
    pLakeShore->switchPowerOff();
    ui->startRvsTButton->setText("Start R vs T");
    ui->startIvsVButton->setEnabled(true);
    qDebug() << "Ramp is Done";
    ui->statusBar->showMessage(QString("Measurements Completed !"));
    return;
  }
}


void
MainWindow::onTimeToReadT() {
  double currentTemperature = pLakeShore->getTemperature();
  currentTime = QDateTime::currentDateTime();
//  qDebug()<< "T = " << currentTemperature;
  pPlotTemperature->NewPoint(iCurrentTPlot,
                             double(startReadingTTime.secsTo(currentTime)),
                             currentTemperature);
  pPlotTemperature->UpdatePlot();
}


void
MainWindow::on_startIvsVButton_clicked() {
  if(ui->startIvsVButton->text().contains("Stop")) {
    ui->startIvsVButton->setText("Start I vs V");
    ui->startRvsTButton->setEnabled(true);
    return;
  }
  //else
  if(configureIvsVDialog.exec() == QDialog::Rejected) return;
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  ui->startRvsTButton->setDisabled(true);
  ui->startIvsVButton->setText("Stop I vs V");

  // Plot of Condicibility vs Temperature
  if(pPlotMeasurements) delete pPlotMeasurements;
  sMeasurementPlotLabel = QString("I [A] vs V [V]");

  pPlotMeasurements = new Plot2D(this, sMeasurementPlotLabel);
  pPlotMeasurements->setMaxPoints(maxPlotPoints);
  pPlotMeasurements->NewDataSet(1,//Id
                                3, //Pen Width
                                QColor(255, 255, 0),// Color
                                StripChart::ipoint,// Symbol
                                "I"// Title
                                );
  pPlotMeasurements->SetShowDataSet(1, true);
  pPlotMeasurements->SetShowTitle(1, true);
  pPlotMeasurements->UpdatePlot();
  pPlotMeasurements->show();
  // Plot of Temperature vs Time
  if(pPlotTemperature) delete pPlotTemperature;
  sTemperaturePlotLabel = QString("T [K] vs t [s]");
  pPlotTemperature = new Plot2D(this, sTemperaturePlotLabel);
  pPlotTemperature->setMaxPoints(maxPlotPoints);
  pPlotTemperature->NewDataSet(1,//Id
                               3, //Pen Width
                               QColor(255, 255, 0),// Color
                               StripChart::ipoint,// Symbol
                               "T"// Title
                               );
  pPlotTemperature->SetShowDataSet(1, true);
  pPlotTemperature->SetShowTitle(1, true);
  pPlotTemperature->UpdatePlot();
  pPlotTemperature->show();

  ui->statusBar->clearMessage();
  QApplication::restoreOverrideCursor();
}


void
MainWindow::onComplianceEvent() {
  qCritical() << "MainWindow::onComplianceEvent()";
  on_startRvsTButton_clicked();
}


void
MainWindow::onKeithleyReadyForTrigger() {
  isK236ReadyForTrigger = true;
}


void
MainWindow::onNewKeithleyReading(QDateTime dataTime, QString sDataRead) {
  if(!bRunning)
    return;
  QStringList sMeasures = QStringList(sDataRead.split(",", QString::SkipEmptyParts));
  if(sMeasures.count() < 2) {
    qDebug() << "MainWindow::onNewKeithleyReading: Measurement Format Error";
    return;
  }
  double currentTemperature = pLakeShore->getTemperature();
  double t = double(startMeasuringTime.msecsTo(dataTime))/1000.0;
  Q_UNUSED(t)
  double current = sMeasures.at(0).toDouble();
  double voltage = sMeasures.at(1).toDouble();
  if(currentLampStatus == LAMP_OFF) {
    pPlotMeasurements->NewPoint(iPlotDark, currentTemperature, current/voltage);
    pPlotMeasurements->UpdatePlot();
    currentLampStatus = LAMP_ON;
  }
  else {
    pPlotMeasurements->NewPoint(iPlotPhoto, currentTemperature, current/voltage);
    pPlotMeasurements->UpdatePlot();
    currentLampStatus = LAMP_OFF;
  }
  DAQmxErrChk(
    DAQmxWriteDigitalLines(
      lampTaskHandle,
      1,                       // numSampsPerChan
      1,                       // autoStart
      1.0,                     // timeout [s]
      DAQmx_Val_GroupByChannel,// dataLayout
      &currentLampStatus,      // writeArray[]
      NULL,                    // *sampsPerChanWritten
      NULL)                    // *reserved
  );
  return;

Error:
  if(DAQmxFailed(error)) {
    char errBuf[2048];
    DAQmxGetExtendedErrorInfo(errBuf, sizeof(errBuf));
    DAQmxClearTask(lampTaskHandle);
    QMessageBox::critical(Q_NULLPTR, "DAQmx Error", QString(errBuf));
  }
  return;
}


bool
MainWindow::getNewSigmaMeasure() {
  if(!isK236ReadyForTrigger)
    return false;
  isK236ReadyForTrigger = false;
  return pKeithley->sendTrigger();
}


int
MainWindow::JunctionCheck() {// Per sapere se abbiamo una giunzione !
  return pKeithley->junctionCheck();
}

