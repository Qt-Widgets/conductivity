#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "utility.h"
#include "keithley236.h"
#include "lakeshore330.h"
#include "stripchart.h"

#include <QMessageBox>
#include <QDebug>
#include <QSettings>
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
  , pKeithley(Q_NULLPTR)
  , pLakeShore(Q_NULLPTR)
  , GpibBoardID(0)
  , pPlot1(Q_NULLPTR)
{
  ui->setupUi(this);

  QSettings settings;
  restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
  restoreState(settings.value("mainWindowState").toByteArray());
/*
  if(!CheckInstruments())
    exit(-1);

  int initError;
  if(pKeithley  != Q_NULLPTR)  {
    initError = pKeithley->Init();
    if(initError) {
      exit(-2);
    }
  }
  if(pLakeShore != Q_NULLPTR) {
    initError = pLakeShore->Init();
    if(initError) {
      exit(-3);
    }
    pLakeShore->SetTemperature(100.0);
  }
*/

  pPlot1 = new StripChart(this, QString("Measurements"));
  pPlot1->setMaxPoints(nChartPoints);
  pPlot1->show();
}


MainWindow::~MainWindow() {
  if(pKeithley != Q_NULLPTR) delete pKeithley;
  pKeithley = Q_NULLPTR;
  if(pPlot1) delete pPlot1;
  pPlot1 = Q_NULLPTR;

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
MainWindow::on_configureButton_clicked() {
  configureDialog.exec();
}
