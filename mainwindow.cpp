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
#include "stripchart.h"

#include <QMessageBox>
#include <QDebug>
#include <QSettings>
#include <QFile>
#include <ni4882.h>
#include <NIDAQmx.h>

/*
#define GPIB_COMMAND_ERROR      -1001
#define SPOLL_ERR               -1000

#define READING_TIMER       1
#define TIMER_TO_START      2
#define TIMER_TO_WAIT       3
#define POSITIVE_STEP_TIMER 4
#define NEGATIVE_STEP_TIMER 5
#define STABILIZE_TIMER     6
#define VOLTAGE_STEP_TIMER  7
#define TIMEOUT_TIMER      10
*/



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
#ifdef SIMULATION
  , sLampLine(QString("Fake2/port0/line0"))
#else
  , sLampLine(QString("NiUSB-6211/port0/line0"))
#endif
  , currentLampStatus(LAMP_OFF)
  , pPlotMeasurements(Q_NULLPTR)
  , pPlotTemperature(Q_NULLPTR)
  , maxChartPoints(3000)
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

  if(ThreadIbsta() & ERR) {
    qDebug() << "In DevClearList: ";
    qDebug() << "Errore IEEE 488\nControllare che gli Strumenti Siano Connessi e Accesi !\n";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return false;
  }

  FindLstn(gpibBoardID, padlist, resultlist, 30);
  if(ThreadIbsta() & ERR) {
    qDebug() << "Errore IEEE 488\nControllare che gli Strumenti Siano Connessi e Accesi !\n";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return false;
  }
  int nDevices = ThreadIbcntl();

  // Identify the instruments connected to the GPIB Bus
  char readBuf[257];
  QString sCommand = "*IDN?\r\n";
  for(int i=0; i<nDevices; i++) {
    Send(gpibBoardID, resultlist[i], sCommand.toUtf8().constData(), sCommand.length(), DABend);
    if(ThreadIbsta() & ERR) {
      qDebug() << "In Send: ";
      QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
      qDebug() << sError;
      return false;
    }
    Receive(gpibBoardID, resultlist[i], readBuf, 256, STOPend);
    if(ThreadIbsta() & ERR) {
      qDebug() << "In Receive: ";
      QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
      qDebug() << sError;
      return false;
    }
    QString sInstrumentID = QString(readBuf);
    // La source Measure Unit K236 non risponde al comando di identificazione !!!!
    if(sInstrumentID.contains("NSDCI", Qt::CaseInsensitive)) {
      if(pKeithley == Q_NULLPTR)
        pKeithley = new Keithley236(gpibBoardID, resultlist[i], this);
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
  DAQmxErrChk (
    DAQmxSetDOOutputDriveType(
      lampTaskHandle,
      sLampLine.toLatin1().constData(),
      DAQmx_Val_ActiveDrive)
  );
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
  DAQmxStopTask(lampTaskHandle);
  DAQmxClearTask(lampTaskHandle);
  lampTaskHandle = Q_NULLPTR;
}


void
MainWindow::on_startRvsTButton_clicked() {
  if(ui->startRvsTButton->text().contains("Stop")) {
    stopDAQ();
    ui->startRvsTButton->setText("Start R vs T");
    ui->startIvsVButton->setEnabled(true);
    return;
  }
  // else
  if(configureRvsTDialog.exec() == QDialog::Rejected) return;

  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  // Start the Digital Output Tasks
  ui->statusBar->showMessage("Checking for the Presence of Lamp Switch");
  if(!startDAQ()) {
    ui->statusBar->showMessage("National Instruments DAQ Board not present");
    QApplication::restoreOverrideCursor();
    return;
  }
#ifndef SIMULATION
  // Are the instruments connectd and ready to start ?
  ui->statusBar->showMessage("Checking for the GPIB Instruments");
  if(!CheckInstruments()) {
    ui->statusBar->showMessage("GPIB Instruments not found");
    QApplication::restoreOverrideCursor();
    return;
  }
  int initError;
  if(pKeithley  != Q_NULLPTR)  {
    ui->statusBar->showMessage("Initializing Keithley 236...");
    initError = pKeithley->Init();
    if(initError) {
      ui->statusBar->showMessage("Unable to Initialize Keithley 236...");
      QApplication::restoreOverrideCursor();
      return;
    }
  }
  if(pLakeShore != Q_NULLPTR) {
    ui->statusBar->showMessage("Initializing LakeShore 330...");
    initError = pLakeShore->Init();
    if(initError) {
      ui->statusBar->showMessage("Unable to Initialize LakeShore 330...");
      QApplication::restoreOverrideCursor();
      return;
    }
    pLakeShore->SetTemperature(100.0);
  }

  // Open the Output file
  ui->statusBar->showMessage("Opening Output file...");
  if(pOutputFile) {
    if(pOutputFile->isOpen())
      pOutputFile->close();
    pOutputFile->deleteLater();
    pOutputFile = Q_NULLPTR;
  }
#endif
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

  ui->startIvsVButton->setDisabled(true);
  ui->startRvsTButton->setText("Stop R vs T");

  ui->statusBar->clearMessage();

  if(pPlotMeasurements) delete pPlotMeasurements;
  sMeasurementPlotLabel = QString("R^-1 [OHM^-1] vs T [K]");
  pPlotMeasurements = new StripChart(this, sMeasurementPlotLabel);
  pPlotMeasurements->setMaxPoints(maxChartPoints);
  pPlotMeasurements->NewDataSet(1,//Id
                                3, //Pen Width
                                QColor(255, 255, 0),// Color
                                StripChart::ipoint,// Symbol
                                "R"// Title
                                );
  pPlotMeasurements->SetShowDataSet(1, true);
  pPlotMeasurements->SetShowTitle(1, true);
  pPlotMeasurements->UpdateChart();

  if(pPlotTemperature) delete pPlotTemperature;
  sTemperaturePlotLabel = QString("T [K] vs t [s]");
  pPlotTemperature = new StripChart(this, sTemperaturePlotLabel);
  pPlotTemperature->setMaxPoints(maxChartPoints);
  pPlotTemperature->NewDataSet(1,//Id
                               3, //Pen Width
                               QColor(255, 255, 0),// Color
                               StripChart::ipoint,// Symbol
                               "T"// Title
                               );
  pPlotTemperature->SetShowDataSet(1, true);
  pPlotTemperature->SetShowTitle(1, true);

  pPlotMeasurements->show();
  pPlotTemperature->show();
  InitVvsT();
  QApplication::restoreOverrideCursor();
}


int
MainWindow::InitVvsT() {
#ifndef SIMULATION
  double dAppliedCurrent = configureRvsTDialog.dSourceValue;
  double dVoltageCompliance = 1.0;
  pKeithley->InitVvsT(dAppliedCurrent, dVoltageCompliance);
  pLakeShore->SetTemperature(configureRvsTDialog.dTempStart);
  pLakeShore->SwitchPowerOn();
#endif
  startWaitingTime = QDateTime::currentDateTime();
  qDebug() << "Starting Time:" << startWaitingTime.toString();
  return 0;
}


void
MainWindow::on_startIvsVButton_clicked() {
  if(ui->startIvsVButton->text().contains("Stop")) {
    ui->startIvsVButton->setText("Start I vs V");
    ui->startRvsTButton->setEnabled(true);
    return;
  }
  //else
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  if(configureIvsVDialog.exec() == QDialog::Rejected) return;
  ui->startRvsTButton->setDisabled(true);
  ui->startIvsVButton->setText("Stop I vs V");

  ui->statusBar->clearMessage();

  pPlotMeasurements->ClearChart();
  pPlotTemperature->ClearChart();
  pPlotMeasurements->show();
  pPlotTemperature->show();
  QApplication::restoreOverrideCursor();
}


int
MainWindow::JunctionCheck() {// Per sapere se abbiamo una giunzione !
  return pKeithley->JunctionCheck();
}

