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

#define NO_JUNCTION         0
#define FORWARD_JUNCTION    1
#define REVERSE_JUNCTION   -1

#define SRQ_DISABLED        0
#define WARNING             1
#define SWEEP_DONE          2
#define TRIGGER_OUT         4
#define READING_DONE        8
#define READY_FOR_TRIGGER  16
#define K236_ERROR         32
#define COMPLIANCE        128

#define READING_TIMER       1
#define TIMER_TO_START      2
#define TIMER_TO_WAIT       3
#define POSITIVE_STEP_TIMER 4
#define NEGATIVE_STEP_TIMER 5
#define STABILIZE_TIMER     6
#define VOLTAGE_STEP_TIMER  7
#define TIMEOUT_TIMER      10
*/



MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , pOutputFile(Q_NULLPTR)
  , pKeithley(Q_NULLPTR)
  , pLakeShore(Q_NULLPTR)
  , GpibBoardID(0)
  , pPlotMeasurements(Q_NULLPTR)
  , pPlotTemperature(Q_NULLPTR)
  , sMeasurementPlotLabel(QString("1/R [OHM] vs T [K]"))
  , sTemperaturePlotLabel(QString("T [K] vs t [s]"))

{
  ui->setupUi(this);

  QSettings settings;
  restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
  restoreState(settings.value("mainWindowState").toByteArray());

  nChartPoints = 3000;
  pPlotMeasurements = new StripChart(this, sMeasurementPlotLabel);
  pPlotMeasurements->setMaxPoints(nChartPoints);
  pPlotMeasurements->show();
  pPlotTemperature = new StripChart(this, sTemperaturePlotLabel);
  pPlotTemperature->setMaxPoints(nChartPoints);
  pPlotTemperature->show();
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


void
MainWindow::on_configureButton_clicked() {
  configureDialog.exec();
}


bool
MainWindow::CheckInstruments() {
  Addr4882_t padlist[31];
  Addr4882_t resultlist[31];
  for(short i=0; i<30; i++) padlist[i] = i+1;
  padlist[30] = NOADDR;

  SendIFC(GpibBoardID);
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
  DevClearList(GpibBoardID, &addrlist);

  if(ThreadIbsta() & ERR) {
    qDebug() << "In DevClearList: ";
    qDebug() << "Errore IEEE 488\nControllare che gli Strumenti Siano Connessi e Accesi !\n";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return false;
  }

  FindLstn(GpibBoardID, padlist, resultlist, 30);
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
    Send(GpibBoardID, resultlist[i], sCommand.toUtf8().constData(), sCommand.length(), DABend);
    if(ThreadIbsta() & ERR) {
      qDebug() << "In Send: ";
      QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
      qDebug() << sError;
      return false;
    }
    Receive(GpibBoardID, resultlist[i], readBuf, 256, STOPend);
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
        pKeithley = new Keithley236(GpibBoardID, resultlist[i], this);
    } else if(sInstrumentID.contains("MODEL330", Qt::CaseInsensitive)) {
      if(pLakeShore == NULL)
        pLakeShore = new LakeShore330(GpibBoardID, resultlist[i], this);
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


void
MainWindow::on_startButton_clicked() {
  // Are the instruments connectd and ready to start ?
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
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
  pOutputFile = new QFile(configureDialog.sBaseDir + "/" + configureDialog.sOutFileName);
  if(!pOutputFile->open(QIODevice::Text|QIODevice::WriteOnly)) {
    QMessageBox::critical(this,
                          "Error: Unable to Open Output File",
                          QString("%1/%2")
                          .arg(configureDialog.sBaseDir)
                          .arg(configureDialog.sOutFileName));
    ui->statusBar->showMessage("Unable to Open Output file...");
    QApplication::restoreOverrideCursor();
    return;
  }
  QApplication::restoreOverrideCursor();
  ui->statusBar->clearMessage();
}
