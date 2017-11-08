#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "utility.h"
#include "keithley236.h"
#include "stripchart.h"

#include <QDebug>
#include <QSettings>
#include <NIDAQmx.h>


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

#define MAX_COMPLIANCE_EVENTS 5


MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , pKeithley(Q_NULLPTR)
  , pPlot1(Q_NULLPTR)
{
  ui->setupUi(this);

  QSettings settings;
  restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
  restoreState(settings.value("mainWindowState").toByteArray());

  pKeithley = new Keithley236(0, 16, this);
  pKeithley->Init();
  ui->statusBar->showMessage("Keythley 236 Source Measure Unit Connected");
  pPlot1 = new StripChart(this, QString("Measurements"));
  pPlot1->setMaxPoints(nChartPoints);

//  plot2 = new StripChart(this, QString("Pitch"));
//  plot2->setMaxPoints(nChartPoints);

//  plot3 = new StripChart(this, QString("Yaw"));
//  plot3->setMaxPoints(nChartPoints);
}


MainWindow::~MainWindow() {
  if(pKeithley != Q_NULLPTR) delete pKeithley;
  pKeithley = Q_NULLPTR;
  if(pPlot1) delete pPlot1;
  pPlot1 = Q_NULLPTR;
//  if(plot2)        delete plot2;       plot2 = NULL;
//  if(plot3)        delete plot3;       plot3 = NULL;
  delete ui;
}


void
MainWindow::closeEvent(QCloseEvent *event) {
  Q_UNUSED(event)
  QSettings settings;
  settings.setValue("mainWindowGeometry", saveGeometry());
  settings.setValue("mainWindowState", saveState());
}
