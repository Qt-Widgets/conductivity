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
#include "cornerstone130.h"
#include "plot2d.h"
#include <qmath.h>

#include <QMessageBox>
#include <QDebug>
#include <QSettings>
#include <QFile>
#include <QThread>
#include <QLayout>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , pOutputFile(Q_NULLPTR)
    , pKeithley(Q_NULLPTR)
    , pLakeShore(Q_NULLPTR)
    , pCornerStone130(Q_NULLPTR)
    , pPlotMeasurements(Q_NULLPTR)
    , pPlotTemperature(Q_NULLPTR)
    , pChartMeasurements(Q_NULLPTR)
    , pChartTemperature(Q_NULLPTR)
    , pDarkMeasurements(Q_NULLPTR)
    , pPhotoMeasurements(Q_NULLPTR)
    , pTemperatures(Q_NULLPTR)
    , pMeasurementsView(Q_NULLPTR)
    , pTemperatureView(Q_NULLPTR)
{
    ui->setupUi(this);
    // Remove the resize-handle in the lower right corner
    ui->statusBar->setSizeGripEnabled(false);
    // Make the size of the window fixed
    setFixedSize(size());
    setWindowIcon(QIcon("qrc:/myLogoT.png"));

    ui->startRvsTButton->show();
    ui->startIvsVButton->show();

    gpibBoardID           = 0;
    presentMeasure        = NoMeasure;
    bRunning              = false;
    currentLampStatus     = LAMP_OFF;
    isK236ReadyForTrigger = false;
    maxPlotPoints         = 3000;

    QSettings settings;
    restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
    restoreState(settings.value("mainWindowState").toByteArray());

initRvsTCharts();// <<<<<<<<<<<<<<<<<<<<<

}


MainWindow::~MainWindow() {
    freeMemory();
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
        if(pLakeShore) pLakeShore->switchPowerOff();
    }
    freeMemory();
}


void
MainWindow::freeMemory() {
    if(pKeithley          != Q_NULLPTR) delete pKeithley;
    if(pLakeShore         != Q_NULLPTR) delete pLakeShore;
    if(pCornerStone130    != Q_NULLPTR) delete pCornerStone130;
    if(pPlotMeasurements  != Q_NULLPTR) delete pPlotMeasurements;
    if(pPlotTemperature   != Q_NULLPTR) delete pPlotTemperature;
    if(pChartMeasurements != Q_NULLPTR) delete pChartMeasurements;
    if(pChartTemperature  != Q_NULLPTR) delete pChartTemperature;
    if(pMeasurementsView  != Q_NULLPTR) delete pMeasurementsView;
    if(pTemperatureView   != Q_NULLPTR) delete pTemperatureView;

    pKeithley          = Q_NULLPTR;
    pLakeShore         = Q_NULLPTR;
    pCornerStone130    = Q_NULLPTR;
    pPlotMeasurements  = Q_NULLPTR;
    pPlotTemperature   = Q_NULLPTR;
    pChartMeasurements = Q_NULLPTR;
    pChartTemperature  = Q_NULLPTR;
    pMeasurementsView  = Q_NULLPTR;
    pTemperatureView   = Q_NULLPTR;
}


bool
MainWindow::checkInstruments() {
    Addr4882_t padlist[31];
    Addr4882_t resultlist[31];
    for(short i=0; i<30; i++) padlist[i] = i+1;
    padlist[30] = NOADDR;

    SendIFC(gpibBoardID);
    if(isGpibError("SendIFC Error. Is the GPIB Interface connected ?"))
        return false;

    // Enable assertion of REN when System Controller
    // Required by the Keithley 236
    ibconfig(gpibBoardID, IbcSRE, 1);
    if(isGpibError("ibconfig() Unable to set REN When SC"))
        return false;

    // If addrlist contains only the constant NOADDR,
    // the Universal Device Clear (DCL) message is sent
    // to all the devices on the bus
    Addr4882_t addrlist;
    addrlist = NOADDR;
    DevClearList(gpibBoardID, &addrlist);
    if(isGpibError("DevClearList() failed. Are the Instruments Connected and Switched On ?"))
        return false;

    FindLstn(gpibBoardID, padlist, resultlist, 30);
    if(isGpibError("FindLstn() failed. Are the Instruments Connected and Switched On ?"))
        return false;
    int nDevices = ThreadIbcnt();
    qInfo() << QString("Found %1 Instruments connected to the GPIB Bus").arg(nDevices);
    // Identify the instruments connected to the GPIB Bus
    QString sCommand, sInstrumentID;
    // Identify the instruments connected to the GPIB Bus
    char readBuf[257];
    int cornerstoneId = 0;
    // Check for the monochromator...
    for(int i=0; i<nDevices; i++) {
        sCommand = "INFO?\r\n";
        Send(gpibBoardID, resultlist[i], sCommand.toUtf8().constData(), sCommand.length(), DABend);
        Receive(gpibBoardID, resultlist[i], readBuf, 256, 0x0A);
        readBuf[ThreadIbcnt()] = '\0';
        sInstrumentID = QString(readBuf);
        qDebug() << QString("Address= %1 - InstrumentID= %2")
                    .arg(resultlist[i])
                    .arg(sInstrumentID);
        if(sInstrumentID.contains("Cornerstone 130", Qt::CaseInsensitive)) {
            cornerstoneId = resultlist[i];
            if(pCornerStone130 == Q_NULLPTR) {
                pCornerStone130 = new CornerStone130(gpibBoardID, resultlist[i], this);
            }
            break;
        }
    }
    if(pCornerStone130 == Q_NULLPTR) {
        QMessageBox::warning(this, "Error", "Cornerstone 130 not Connected",
                             QMessageBox::Abort, QMessageBox::Abort);
        return false;
    }

    // Check for the temperature controller...
    sCommand = "*IDN?\r\n";
    int lakeShoreID = 0;
    for(int i=0; i<nDevices; i++) {
        if(resultlist[i] == cornerstoneId) continue;
        Send(gpibBoardID, resultlist[i], sCommand.toUtf8().constData(), sCommand.length(), DABend);
        Receive(gpibBoardID, resultlist[i], readBuf, 256, STOPend);
        readBuf[ThreadIbcnt()] = '\0';
        sInstrumentID = QString(readBuf);
        qDebug() << QString("Address= %1 - InstrumentID= %2")
                    .arg(resultlist[i])
                    .arg(sInstrumentID);
        if(sInstrumentID.contains("MODEL330", Qt::CaseInsensitive)) {
            lakeShoreID = resultlist[i];
            if(pLakeShore == NULL) {
                pLakeShore = new LakeShore330(gpibBoardID, resultlist[i], this);
            }
            break;
        }
    }
    if(pLakeShore == Q_NULLPTR) {
        QMessageBox::warning(this, "Error", "Lake Shore 330 not Connected",
                             QMessageBox::Abort, QMessageBox::Abort);
        return false;
    }

    // Check for the Keithley 236
    sCommand = "U0X";
    for(int i=0; i<nDevices; i++) {
        if(resultlist[i] == cornerstoneId) continue;
        if(resultlist[i] == lakeShoreID) continue;
        Send(gpibBoardID, resultlist[i], sCommand.toUtf8().constData(), sCommand.length(), DABend);
        Receive(gpibBoardID, resultlist[i], readBuf, 256, STOPend);
        readBuf[ThreadIbcnt()] = '\0';
        sInstrumentID = QString(readBuf);
        qDebug() << QString("Address= %1 - InstrumentID= %2")
                    .arg(resultlist[i])
                    .arg(sInstrumentID);
        if(sInstrumentID.contains("236", Qt::CaseInsensitive)) {
            if(pKeithley == Q_NULLPTR) {
                pKeithley = new Keithley236(gpibBoardID, resultlist[i], this);
            }
            break;
        }
    }
    if(pKeithley == Q_NULLPTR) {
        QMessageBox::warning(this, "Error", "Source Measure Unit not Connected",
                             QMessageBox::Abort, QMessageBox::Abort);
        return false;
    }

    return true;
}


bool
MainWindow::switchLampOn() {
    ui->photoButton->setChecked(true);
    return pCornerStone130->openShutter();
}


bool
MainWindow::switchLampOff() {
    ui->photoButton->setChecked(false);
    return pCornerStone130->closeShutter();
}


void
MainWindow::stopRvsT() {
    bRunning = false;
    presentMeasure = NoMeasure;
    waitingTStartTimer.stop();
    stabilizingTimer.stop();
    readingTTimer.stop();
    measuringTimer.stop();
    disconnect(&waitingTStartTimer, 0, 0, 0);
    disconnect(&stabilizingTimer, 0, 0, 0);
    disconnect(&readingTTimer, 0, 0, 0);
    disconnect(&measuringTimer, 0, 0, 0);
    if(pOutputFile) {
        pOutputFile->close();
        pOutputFile->deleteLater();
        pOutputFile = Q_NULLPTR;
    }
    if(pKeithley != Q_NULLPTR) {
        disconnect(pKeithley, 0, 0, 0);
        pKeithley->endVvsT();
        pKeithley->deleteLater();
        pKeithley = Q_NULLPTR;
    }
    if(pLakeShore != Q_NULLPTR) {
        disconnect(pLakeShore, 0, 0, 0);
        pLakeShore->switchPowerOff();
        pLakeShore->deleteLater();
        pLakeShore = Q_NULLPTR;
    }
    switchLampOff();

    ui->endTimeEdit->clear();
    ui->startRvsTButton->setText("Start R vs T");
    ui->startIvsVButton->setEnabled(true);
    QApplication::restoreOverrideCursor();
}


void
MainWindow::on_startRvsTButton_clicked() {
    if(ui->startRvsTButton->text().contains("Stop")) {
        stopRvsT();
        ui->statusBar->showMessage("Measure (R vs T) Halted");
        return;
    }
    // else
    if(configureRvsTDialog.exec() == QDialog::Rejected)
        return;

    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

    // Are the GPIB instruments connectd and ready to start ?
    ui->statusBar->showMessage("Checking for the GPIB Instruments");
    if(!checkInstruments()) {
        ui->statusBar->showMessage("GPIB Instruments not found");
        QApplication::restoreOverrideCursor();
        return;
    }
    //Initializing Corner Stone 130
    ui->statusBar->showMessage("Initializing Corner Stone 130...");
    if(pCornerStone130->init() != NO_ERROR){
        ui->statusBar->showMessage("Unable to Initialize Corner Stone 130...");
        QApplication::restoreOverrideCursor();
        return;
    }
    switchLampOff();
    pCornerStone130->setGrating(configureRvsTDialog.iGratingNumber);
    pCornerStone130->setWavelength(configureRvsTDialog.dWavelength);
    // Initializing Keithley 236
    ui->statusBar->showMessage("Initializing Keithley 236...");
    if(pKeithley->init()) {
        ui->statusBar->showMessage("Unable to Initialize Keithley 236...");
        QApplication::restoreOverrideCursor();
        return;
    }
    isK236ReadyForTrigger = false;
    connect(pKeithley, SIGNAL(complianceEvent()),
            this, SLOT(onComplianceEvent()));
    connect(pKeithley, SIGNAL(readyForTrigger()),
            this, SLOT(onKeithleyReadyForTrigger()));
    connect(pKeithley, SIGNAL(newReading(QDateTime, QString)),
            this, SLOT(onNewKeithleyReading(QDateTime, QString)));
    // Initializing LakeShore 330
    ui->statusBar->showMessage("Initializing LakeShore 330...");
    if(pLakeShore->init()) {
        ui->statusBar->showMessage("Unable to Initialize LakeShore 330...");
        QApplication::restoreOverrideCursor();
        return;
    }
    // Open the Output file
    ui->statusBar->showMessage("Opening Output file...");
    if(!prepareOutputFile(configureRvsTDialog.sBaseDir,
                          configureRvsTDialog.sOutFileName))
    {
        ui->statusBar->showMessage("Unable to Open the Output file...");
        QApplication::restoreOverrideCursor();
        return;
    }
    // Write the header
    pOutputFile->write(QString("%1 %2 %3 %4 %5 %6")
                       .arg("T-Dark[K]", 12)
                       .arg("V-Dark[V]", 12)
                       .arg("I-Dark[A]", 12)
                       .arg("T-Photo[K]", 12)
                       .arg("V-Photo[V]", 12)
                       .arg("I-Photo[A]\n", 12)
                       .toLocal8Bit());
    pOutputFile->write(configureRvsTDialog.sSampleInfo.toLocal8Bit());
    pOutputFile->write("\n");
    pOutputFile->write(QString("Grating #= %1 Wavelength = %2 nm")
                               .arg(configureRvsTDialog.iGratingNumber)
                               .arg(configureRvsTDialog.dWavelength).toLocal8Bit());
    pOutputFile->write("\n");
    pOutputFile->flush();
    // Init the Plots
    initRvsTPlots();
    initRvsTCharts();
    // Configure Thermostat
    pLakeShore->setTemperature(configureRvsTDialog.dTempStart);
    pLakeShore->switchPowerOn(3);
    // Configure Source-Measure Unit
    double dCompliance = configureRvsTDialog.dCompliance;
    if(configureRvsTDialog.bSourceI) {
        presentMeasure = RvsTSourceI;
        double dAppliedCurrent = configureRvsTDialog.dSourceValue;
        pKeithley->initVvsTSourceI(dAppliedCurrent, dCompliance);
    }
    else {
        presentMeasure = RvsTSourceV;
        double dAppliedVoltage = configureRvsTDialog.dSourceValue;
        pKeithley->initVvsTSourceV(dAppliedVoltage, dCompliance);
    }
    // Configure the needed timers
    connect(&waitingTStartTimer, SIGNAL(timeout()),
            this, SLOT(onTimeToCheckReachedT()));
    connect(&readingTTimer, SIGNAL(timeout()),
            this, SLOT(onTimeToReadT()));
    waitingTStartTime = QDateTime::currentDateTime();
    // Read and plot initial value of Temperature
    startReadingTTime = waitingTStartTime;
    onTimeToReadT();
    readingTTimer.start(5000);
    // Start the reaching of the Initial Temperature
    waitingTStartTimer.start(5000);

    // All done... compute the time needed for the measurement:
    startMeasuringTime = QDateTime::currentDateTime();
    double deltaT, expectedMinutes;
    deltaT = configureRvsTDialog.dTempEnd -
             configureRvsTDialog.dTempStart;
    expectedMinutes = deltaT / configureRvsTDialog.dTRate +
                      configureRvsTDialog.iReachingTime +
                      configureRvsTDialog.iStabilizingTime;
    endMeasureTime = startMeasuringTime.addSecs(expectedMinutes*60.0);
    QString sString = endMeasureTime.toString("hh:mm dd-MM-yyyy");
    ui->endTimeEdit->setText(sString);

    // now we are waiting for reaching the initial temperature
    ui->startIvsVButton->setDisabled(true);
    ui->startRvsTButton->setText("Stop R vs T");
    ui->statusBar->showMessage(QString("%1 Waiting Initial T[%2K]")
                               .arg(waitingTStartTime.toString())
                               .arg(configureRvsTDialog.dTempStart));
}


void
MainWindow::on_startIvsVButton_clicked() {
    if(ui->startIvsVButton->text().contains("Stop")) {
        stopIvsV();
        ui->statusBar->showMessage("Measure (I vs V) Halted");
        return;
    }
    //else
    if(configureIvsVDialog.exec() == QDialog::Rejected)
        return;

    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

    // Are the GPIB instruments connectd and ready to start ?
    ui->statusBar->showMessage("Checking for the GPIB Instruments");
    if(!checkInstruments()) {
        ui->statusBar->showMessage("GPIB Instruments not found");
        stopIvsV();
        return;
    }
    // Initializing Keithley 236
    ui->statusBar->showMessage("Initializing Keithley 236...");
    if(pKeithley->init()) {
        ui->statusBar->showMessage("Unable to Initialize Keithley 236...");
        stopIvsV();
        return;
    }
    isK236ReadyForTrigger = false;
    connect(pKeithley, SIGNAL(complianceEvent()),
            this, SLOT(onComplianceEvent()));
    connect(pKeithley, SIGNAL(readyForTrigger()),
            this, SLOT(onKeithleyReadyForSweepTrigger()));
    // Initializing LakeShore 330
    ui->statusBar->showMessage("Initializing LakeShore 330...");
    if(pLakeShore->init()) {
        ui->statusBar->showMessage("Unable to Initialize LakeShore 330...");
        stopIvsV();
        return;
    }
    // Open the Output file
    ui->statusBar->showMessage("Opening Output file...");
    if(!prepareOutputFile(configureIvsVDialog.sBaseDir,
                          configureIvsVDialog.sOutFileName))
    {
        stopIvsV();
        return;
    }
    pOutputFile->write(QString("%1 %2 %3\n")
                       .arg("Voltage[V]", 12)
                       .arg("Current[A]", 12)
                       .arg("Temp.[K]", 12).toLocal8Bit());
    pOutputFile->write(configureIvsVDialog.sSampleInfo.toLocal8Bit());
    pOutputFile->write("\n");
    pOutputFile->flush();

    ui->statusBar->showMessage("Checking the presence of a Junction...");
    double vStart = configureIvsVDialog.dVStart;
    double vStop =  configureIvsVDialog.dVStop;
    junctionDirection = pKeithley->junctionCheck(vStart, vStop);
    if(junctionDirection == pKeithley->ERROR_JUNCTION) {
        ui->statusBar->showMessage("Error Checking the presence of a Junction...");
        stopIvsV();
        return;
    }
    // Now we know how to proceed... (maybe...)
    initIvsVPlots();
    initIvsVCharts();
    isK236ReadyForTrigger = false;
    presentMeasure = IvsV;
    connect(&readingTTimer, SIGNAL(timeout()),
            this, SLOT(onTimeToReadT()));
    readingTTimer.start(5000);
    // Read and plot initial value of Temperature
    startReadingTTime = QDateTime::currentDateTime();
    onTimeToReadT();
    double expectedSeconds;
    startMeasuringTime = QDateTime::currentDateTime();
    expectedSeconds = 0.32+configureIvsVDialog.iWaitTime/1000.0;
    expectedSeconds *= 4.0 * configureIvsVDialog.iNSweepPoints;
    if(configureIvsVDialog.bUseThermostat) {
        connect(&waitingTStartTimer, SIGNAL(timeout()),
                this, SLOT(onTimeToCheckT()));
        waitingTStartTime = QDateTime::currentDateTime();
        // Start the reaching of the Initial Temperature
        // Configure Thermostat
        setPointT = configureIvsVDialog.dTStart;
        pLakeShore->setTemperature(setPointT);
        pLakeShore->switchPowerOn(3);
        waitingTStartTimer.start(5000);
        ui->statusBar->showMessage(QString("%1 Waiting Initial T[%2K]")
                                   .arg(waitingTStartTime.toString())
                                   .arg(configureIvsVDialog.dTStart));
        // All done... compute the time needed for the measurement:
        double deltaT;
        deltaT = configureIvsVDialog.dTStop -
                 configureIvsVDialog.dTStart;
        expectedSeconds += 60.0 *(configureIvsVDialog.iReachingTStart +
                                  configureIvsVDialog.iTimeToSteadyT);
        expectedSeconds *= int(deltaT / configureIvsVDialog.dTStep);
    }
    else {
        startI_V();
    }
    endMeasureTime = startMeasuringTime.addSecs(expectedSeconds);
    QString sString = endMeasureTime.toString("hh:mm:ss dd-MM-yyyy");
    ui->endTimeEdit->setText(sString);
    ui->startRvsTButton->setDisabled(true);
    ui->startIvsVButton->setText("Stop I vs V");
}


void
MainWindow::startI_V() {
    if(junctionDirection == 0) {
        // No diode junction
        qDebug() << "No junctions in device";
        ui->statusBar->showMessage("No junctions: Sweeping...Please Wait");
        double dIStart = configureIvsVDialog.dIStart;
        double dIStop = configureIvsVDialog.dIStop;
        int nSweepPoints = configureIvsVDialog.iNSweepPoints;
        double dIStep = qAbs(dIStop - dIStart) / double(nSweepPoints);
        double dDelayms = double(configureIvsVDialog.iWaitTime);
        double dCompliance = qMax(qAbs(configureIvsVDialog.dVStart),
                                  qAbs(configureIvsVDialog.dVStop));
        presentMeasure = IvsVSourceI;
        connect(pKeithley, SIGNAL(sweepDone(QDateTime,QString)),
                this, SLOT(onKeithleySweepDone(QDateTime, QString)));
        pKeithley->initISweep(dIStart, dIStop, dIStep, dDelayms, dCompliance);
    }
    else if(junctionDirection > 0) {// Forward junction
//        qDebug() << "Forward Direction Handling";
        ui->statusBar->showMessage("Forward junction: Sweeping...Please Wait");
        double dIStart = 0.0;
        double dIStop = configureIvsVDialog.dIStop;
        int nSweepPoints = configureIvsVDialog.iNSweepPoints;
        double dIStep = qAbs(dIStop - dIStart) / double(nSweepPoints);
        double dDelayms = double(configureIvsVDialog.iWaitTime);
        double dCompliance = qMax(qAbs(configureIvsVDialog.dVStart),
                                  qAbs(configureIvsVDialog.dVStop));
        presentMeasure = IvsVSourceI;
        connect(pKeithley, SIGNAL(sweepDone(QDateTime,QString)),
                this, SLOT(onIForwardSweepDone(QDateTime,QString)));
        pKeithley->initISweep(dIStart, dIStop, dIStep, dDelayms, dCompliance);
    }
    else {// Reverse junction
//        qDebug() << "Reverse Direction Handling";
        ui->statusBar->showMessage("Reverse junction: Sweeping...Please Wait");
        double dVStart = 0.0;
        double dVStop = configureIvsVDialog.dVStop;
        int nSweepPoints = configureIvsVDialog.iNSweepPoints;
        double dVStep = qAbs(dVStop - dVStart) / double(nSweepPoints);
        double dDelayms = double(configureIvsVDialog.iWaitTime);
        double dCompliance = qMax(qAbs(configureIvsVDialog.dIStart),
                                  qAbs(configureIvsVDialog.dIStop));
        presentMeasure = IvsVSourceV;
        connect(pKeithley, SIGNAL(sweepDone(QDateTime,QString)),
                this, SLOT(onVReverseSweepDone(QDateTime,QString)));
        pKeithley->initVSweep(dVStart, dVStop, dVStep, dDelayms, dCompliance);
    }
}


void
MainWindow::stopIvsV() {
    presentMeasure = NoMeasure;
    if(pOutputFile) {
        pOutputFile->close();
        pOutputFile->deleteLater();
        pOutputFile = Q_NULLPTR;
    }
    readingTTimer.stop();
    waitingTStartTimer.stop();
    stabilizingTimer.stop();
    disconnect(&readingTTimer, 0, 0, 0);
    disconnect(&waitingTStartTimer, 0, 0, 0);
    disconnect(&stabilizingTimer, 0, 0, 0);
    if(pKeithley != Q_NULLPTR) {
        disconnect(pKeithley, 0, 0, 0);
        pKeithley->stopSweep();
        pKeithley->deleteLater();
        pKeithley = Q_NULLPTR;
    }
    if(pLakeShore != Q_NULLPTR) {
        disconnect(pLakeShore, 0, 0, 0);
        pLakeShore->switchPowerOff();
        pLakeShore->deleteLater();
        pLakeShore = Q_NULLPTR;
    }
    ui->endTimeEdit->clear();
    ui->startIvsVButton->setText("Start I vs V");
    ui->startRvsTButton->setEnabled(true);
    QApplication::restoreOverrideCursor();
}


bool
MainWindow::prepareOutputFile(QString sBaseDir, QString sFileName) {
    if(pOutputFile) {
        pOutputFile->close();
        pOutputFile->deleteLater();
        pOutputFile = Q_NULLPTR;
    }
    pOutputFile = new QFile(sBaseDir + "/" + sFileName);
    if(!pOutputFile->open(QIODevice::Text|QIODevice::WriteOnly)) {
        QMessageBox::critical(this,
                              "Error: Unable to Open Output File",
                              QString("%1/%2")
                              .arg(sBaseDir)
                              .arg(sFileName));
        ui->statusBar->showMessage("Unable to Open Output file...");
        return false;
    }
    return true;
}


void
MainWindow::initRvsTPlots() {
    if(pPlotMeasurements) delete pPlotMeasurements;
    if(pPlotTemperature) delete pPlotTemperature;
    pPlotMeasurements = Q_NULLPTR;
    pPlotTemperature = Q_NULLPTR;
    // Plot of Conductivity vs Temperature
    sMeasurementPlotLabel = QString("log(S) [Ohm^-1] -vs- 1000/T [K^-1]");
    pPlotMeasurements = new Plot2D(this, sMeasurementPlotLabel);
    pPlotMeasurements->setMaxPoints(maxPlotPoints);
    pPlotMeasurements->SetLimits(0.0, 1.0, 0.1, 1.0, true, true, false, true);

    pPlotMeasurements->NewDataSet(iPlotDark,//Id
                                  3, //Pen Width
                                  QColor(255, 0, 0),// Color
                                  Plot2D::ipoint,// Symbol
                                  "Dark"// Title
                                  );
    pPlotMeasurements->SetShowDataSet(iPlotDark, true);
    pPlotMeasurements->SetShowTitle(iPlotDark, true);

    pPlotMeasurements->NewDataSet(iPlotPhoto,//Id
                                  3, //Pen Width
                                  QColor(255, 255, 0),// Color
                                  Plot2D::ipoint,// Symbol
                                  "Photo"// Title
                                  );
    pPlotMeasurements->SetShowDataSet(iPlotPhoto, true);
    pPlotMeasurements->SetShowTitle(iPlotPhoto, true);

    pPlotMeasurements->UpdatePlot();
    pPlotMeasurements->show();

    // Plot of Temperature vs Time
    sTemperaturePlotLabel = QString("T [K] vs t [s]");
    pPlotTemperature = new Plot2D(this, sTemperaturePlotLabel);
    pPlotTemperature->setMaxPoints(maxPlotPoints);
    pPlotTemperature->SetLimits(0.0, 1.0, 0.0, 1.0, true, true, false, false);

    pPlotTemperature->NewDataSet(1,//Id
                                 3, //Pen Width
                                 QColor(255, 0, 0),// Color
                                 Plot2D::ipoint,// Symbol
                                 "T"// Title
                                 );
    pPlotTemperature->SetShowDataSet(1, true);
    pPlotTemperature->SetShowTitle(1, true);

    pPlotTemperature->UpdatePlot();
    pPlotTemperature->show();
    iCurrentTPlot = 1;
}



void
MainWindow::initRvsTCharts() {
    if(pChartMeasurements) delete pChartMeasurements;
    if(pChartTemperature)  delete pChartTemperature;
    if(pDarkMeasurements)  delete pDarkMeasurements;
    if(pPhotoMeasurements) delete pPhotoMeasurements;
    if(pMeasurementsView)  delete pMeasurementsView;
    if(pTemperatures)      delete pTemperatures;
    if(pTemperatureView)   delete pTemperatureView;

    pChartMeasurements = Q_NULLPTR;
    pPlotTemperature   = Q_NULLPTR;
    pDarkMeasurements  = Q_NULLPTR;
    pPhotoMeasurements = Q_NULLPTR;
    pMeasurementsView  = Q_NULLPTR;
    pTemperatures      = Q_NULLPTR;
    pTemperatureView   = Q_NULLPTR;

    xDataMin = configureRvsTDialog.dTempStart;
    xDataMax = configureRvsTDialog.dTempEnd;
    yDataMin = 1.0e-1;
    yDataMax = 1.0e+1;

    xTempMin = 0.0;
    xTempMax = 30.0;
    yTempMin = configureRvsTDialog.dTempStart;
    yTempMax = configureRvsTDialog.dTempEnd;;

    // Plot of Conductivity vs Temperature
    sMeasurementPlotLabel = QString("log(S) [Ohm^-1] -vs- 1000/T [K^-1]");
    pChartMeasurements = new QChart();
    pChartMeasurements->setTheme(QChart::ChartThemeBlueCerulean);
    pChartMeasurements->setAnimationOptions(QChart::SeriesAnimations);
    pChartMeasurements->setTitle(sMeasurementPlotLabel);
    // Data in Dark
    pDarkMeasurements = new QScatterSeries();
    pDarkMeasurements->setColor(QColor(255, 0, 0, 255));
    pChartMeasurements->addSeries(pDarkMeasurements);
    // Data in Photo
    pPhotoMeasurements = new QScatterSeries();
    pPhotoMeasurements->setColor(QColor(255, 255, 0, 255));
    pChartMeasurements->addSeries(pPhotoMeasurements);
    // X Axis
    QValueAxis *xAxis = new QValueAxis();
    xAxis->setTickCount(11);
    xAxis->setLabelFormat("%.1f");
    xAxis->setShadesVisible(false);
    xAxis->setShadesBrush(QBrush(QColor(249, 249, 255)));
    xAxis->setRange(xDataMin, xDataMax);
    // Y Axis
    QLogValueAxis *yAxis = new QLogValueAxis();
    yAxis->setBase(10.0);
    yAxis->setLabelFormat("%.1g");
    yAxis->setShadesVisible(false);
    yAxis->setShadesBrush(QBrush(QColor(249, 249, 255)));
    yAxis->setGridLineVisible(true);
    yAxis->setMinorGridLineVisible(true);
    yAxis->setRange(yDataMin, yDataMax);
    // Add Axes to Plot
    pChartMeasurements->addAxis(xAxis, Qt::AlignBottom);
    pChartMeasurements->addAxis(yAxis, Qt::AlignLeft);
    pDarkMeasurements->attachAxis(xAxis);
    pDarkMeasurements->attachAxis(yAxis);
    pPhotoMeasurements->attachAxis(xAxis);
    pPhotoMeasurements->attachAxis(yAxis);
    // The Plot View
    pMeasurementsView = new QChartView();
//    Qt::WindowFlags flags = pMeasurementsView->windowFlags();
//    flags = Qt::CustomizeWindowHint;
//    flags |= Qt::WindowMinMaxButtonsHint;
//    flags &= ~Qt::WindowContextHelpButtonHint;
//    flags &= ~Qt::WindowCloseButtonHint;
//    pMeasurementsView->setWindowFlags(flags);

    pMeasurementsView->setChart(pChartMeasurements);
    pMeasurementsView->setRenderHint(QPainter::Antialiasing);
    pMeasurementsView->setWindowTitle(sMeasurementPlotLabel);
    pMeasurementsView->setRubberBand(QChartView::RectangleRubberBand);

    // Plot of Temperature vs Time
    sTemperaturePlotLabel = QString("T [K] vs t [s]");
    pChartTemperature = new QChart();
    pChartTemperature->setTheme(QChart::ChartThemeBlueCerulean);
    pChartTemperature->setAnimationOptions(QChart::SeriesAnimations);
    pChartTemperature->setTitle(sTemperaturePlotLabel);
    // Data
    pTemperatures = new QLineSeries();
    pTemperatures->setColor(QColor(255, 0, 0, 255));
    pChartTemperature->addSeries(pTemperatures);
    // X Axis
    QValueAxis *xAxisT = new QValueAxis();
    xAxisT->setTickCount(11);
    xAxisT->setLabelFormat("%.1f");
    xAxisT->setShadesVisible(false);
    xAxisT->setShadesBrush(QBrush(QColor(249, 249, 255)));
    xAxisT->setRange(xTempMin, xTempMax);
    // Y Axis
    QValueAxis *yAxisT = new QValueAxis();
    yAxisT->setTickCount(11);
    yAxisT->setLabelFormat("%.1f");
    yAxisT->setShadesVisible(false);
    yAxisT->setShadesBrush(QBrush(QColor(249, 249, 255)));
    yAxisT->setMinorTickCount(-1);
    yAxisT->setRange(yTempMin, yTempMax);
    // Add Axes to Plot
    pChartTemperature->addAxis(xAxisT, Qt::AlignBottom);
    pChartTemperature->addAxis(yAxisT, Qt::AlignLeft);
    // Data
    pTemperatures->attachAxis(xAxisT);
    pTemperatures->attachAxis(yAxisT);
    // The Plot View
    pTemperatureView = new QChartView();
    pTemperatureView->setChart(pChartTemperature);
    pTemperatureView->setRenderHint(QPainter::Antialiasing);
    pTemperatureView->setWindowTitle(sTemperaturePlotLabel);
    pTemperatureView->setRubberBand(QChartView::RectangleRubberBand);
    // Show the Charts
    pMeasurementsView->show();
    pTemperatureView->show();
}



void
MainWindow::initIvsVPlots() {
    if(pPlotMeasurements) delete pPlotMeasurements;
    pPlotMeasurements = Q_NULLPTR;
    if(pPlotTemperature) delete pPlotTemperature;
    pPlotTemperature = Q_NULLPTR;
    // Plot of Current vs Voltage
    sMeasurementPlotLabel = QString("I [A] vs V [V]");

    pPlotMeasurements = new Plot2D(this, sMeasurementPlotLabel);
    pPlotMeasurements->setMaxPoints(maxPlotPoints);
    pPlotMeasurements->NewDataSet(1,//Id
                                  3, //Pen Width
                                  QColor(255, 255, 0),// Color
                                  Plot2D::ipoint,// Symbol
                                  "IvsV"// Title
                                  );
    pPlotMeasurements->SetShowDataSet(1, true);
    pPlotMeasurements->SetShowTitle(1, true);
    pPlotMeasurements->SetLimits(0.0, 1.0, 0.0, 1.0, true, true, false, false);
    pPlotMeasurements->UpdatePlot();
    pPlotMeasurements->show();
    // Plot of Temperature vs Time
    sTemperaturePlotLabel = QString("T [K] vs t [s]");
    pPlotTemperature = new Plot2D(this, sTemperaturePlotLabel);
    pPlotTemperature->setMaxPoints(maxPlotPoints);
    pPlotTemperature->NewDataSet(1,//Id
                                 3, //Pen Width
                                 QColor(255, 255, 0),// Color
                                 Plot2D::ipoint,// Symbol
                                 "T"// Title
                                 );
    pPlotTemperature->SetShowDataSet(1, true);
    pPlotTemperature->SetShowTitle(1, true);
    pPlotTemperature->SetLimits(0.0, 1.0, 0.0, 1.0, true, true, false, false);
    pPlotTemperature->UpdatePlot();
    pPlotTemperature->show();
    iCurrentTPlot = 1;
}


void
MainWindow::initIvsVCharts() {
}


// Invoked to check the reaching of the temperature
// Set Point during I-V measurements
void
MainWindow::onTimeToCheckT() {
    double T = pLakeShore->getTemperature();
    if(fabs(T-setPointT) < 0.15) {
        waitingTStartTimer.stop();
        disconnect(&waitingTStartTimer, 0, 0, 0);
        connect(&stabilizingTimer, SIGNAL(timeout()),
                this, SLOT(onSteadyTReached()));
        stabilizingTimer.start(configureIvsVDialog.iTimeToSteadyT*60*1000);
        ui->statusBar->showMessage(QString("T Reached: Thermal Stabilization for %1 min.")
                                   .arg(configureIvsVDialog.iTimeToSteadyT));
    }
    else {
        currentTime = QDateTime::currentDateTime();
        quint64 elapsedSec = waitingTStartTime.secsTo(currentTime);
        if(elapsedSec > quint64(configureIvsVDialog.iReachingTStart)*60) {
            waitingTStartTimer.stop();
            disconnect(&waitingTStartTimer, 0, 0, 0);
            connect(&stabilizingTimer, SIGNAL(timeout()),
                    this, SLOT(onSteadyTReached()));
            stabilizingTimer.start(configureIvsVDialog.iTimeToSteadyT*60*1000);
            ui->statusBar->showMessage(QString("T Reached: Thermal Stabilization for %1 min.")
                                       .arg(configureIvsVDialog.iTimeToSteadyT));
        }
    }
}


// Invoked when the thermal stabilization is done
// during I-V measurements
void
MainWindow::onSteadyTReached() {
    stabilizingTimer.stop();
    disconnect(&stabilizingTimer, 0, 0, 0);
    startI_V();
}


// Invoked to check the reaching of the initial temperature
// Set Point during R vs T measurements
void
MainWindow::onTimeToCheckReachedT() {
    double T = pLakeShore->getTemperature();
    if(fabs(T-configureRvsTDialog.dTempStart) < 0.15) {
        disconnect(&waitingTStartTimer, 0, 0, 0);
        waitingTStartTimer.stop();
        connect(&stabilizingTimer, SIGNAL(timeout()),
                this, SLOT(onTimerStabilizeT()));
        stabilizingTimer.start(configureRvsTDialog.iStabilizingTime*60*1000);
        //      qDebug() << QString("Starting T Reached: Thermal Stabilization...");
        ui->statusBar->showMessage(QString("Starting T Reached: Thermal Stabilization for %1 min.").arg(configureRvsTDialog.iStabilizingTime));
        // Compute the new time needed for the measurement:
        startMeasuringTime = QDateTime::currentDateTime();
        double deltaT, expectedMinutes;
        deltaT = configureRvsTDialog.dTempEnd -
                 configureRvsTDialog.dTempStart;
        expectedMinutes = deltaT / configureRvsTDialog.dTRate +
                          configureRvsTDialog.iStabilizingTime;
        endMeasureTime = startMeasuringTime.addSecs(expectedMinutes*60.0);
        QString sString = endMeasureTime.toString("hh:mm dd-MM-yyyy");
        ui->endTimeEdit->setText(sString);
    }
    else {
        currentTime = QDateTime::currentDateTime();
        quint64 elapsedMsec = waitingTStartTime.secsTo(currentTime);
        //        qDebug() << "Elapsed:" << elapsedMsec << "Maximum:" << quint64(configureRvsTDialog.iReachingTime)*60*1000;
        if(elapsedMsec > quint64(configureRvsTDialog.iReachingTime)*60*1000) {
            disconnect(&waitingTStartTimer, 0, 0, 0);
            waitingTStartTimer.stop();
            connect(&stabilizingTimer, SIGNAL(timeout()),
                    this, SLOT(onTimerStabilizeT()));
            stabilizingTimer.start(configureRvsTDialog.iStabilizingTime*60*1000);
            //            qDebug() << QString("Max Reaching Time Exceed...Thermal Stabilization...");
            ui->statusBar->showMessage(QString("Max Reaching Time Exceed...Thermal Stabilization for %1 min.").arg(configureRvsTDialog.iStabilizingTime));
        }
    }
}


void
MainWindow::onTimerStabilizeT() {
    // It's time to start measurements
    stabilizingTimer.stop();
    disconnect(&stabilizingTimer, 0, 0, 0);
    pPlotTemperature->NewDataSet(2,//Id
                                 3, //Pen Width
                                 QColor(255, 255, 0),// Color
                                 Plot2D::ipoint,// Symbol
                                 "Tm"// Title
                                 );
    pPlotTemperature->SetShowDataSet(2, true);
    pPlotTemperature->SetShowTitle(2, true);
    pPlotTemperature->UpdatePlot();
    iCurrentTPlot = 2;
    //  qDebug() << "Thermal Stabilization Reached: Measure Started";
    ui->statusBar->showMessage(QString("Thermal Stabilization Reached: Measure Started"));
    connect(&measuringTimer, SIGNAL(timeout()),
            this, SLOT(onTimeToGetNewMeasure()));
    if(!pLakeShore->startRamp(configureRvsTDialog.dTempEnd, configureRvsTDialog.dTRate)) {
        ui->statusBar->showMessage(QString("Error Starting the Measure"));
        return;
    }
    double timeBetweenMeasurements = configureRvsTDialog.dInterval*1000.0;
    measuringTimer.start(timeBetweenMeasurements);
    bRunning = true;
}


void
MainWindow::onTimeToGetNewMeasure() {
    getNewMeasure();
    if(!pLakeShore->isRamping()) {// Ramp is Done
        stopRvsT();
        //    qDebug() << "End Temperature Reached: Measure is Done";
        ui->statusBar->showMessage(QString("Measurements Completed !"));
        return;
    }
}


void
MainWindow::onTimeToReadT() {
    double currentTemperature = pLakeShore->getTemperature();
    currentTime = QDateTime::currentDateTime();
    ui->temperatureEdit->setText(QString("%1").arg(currentTemperature));
//    pPlotTemperature->NewPoint(iCurrentTPlot,
//                               double(startReadingTTime.secsTo(currentTime)),
//                               currentTemperature);
//    pPlotTemperature->UpdatePlot();
    double now = double(startReadingTTime.secsTo(currentTime));
    if(now < xTempMin)
        xTempMin = now;
    if(now > xTempMax)
        xTempMax = now;
    if(currentTemperature < yTempMin)
        yTempMin = currentTemperature;
    if(currentTemperature > yTempMax)
        yTempMax = currentTemperature;

    pChartTemperature->axisX()->setRange(xTempMin, xTempMax);
    pChartTemperature->axisY()->setRange(yTempMin, yTempMax);

    pTemperatures->append(now, currentTemperature);
    qDebug() << double(startReadingTTime.secsTo(currentTime))
             << currentTemperature;
}


void
MainWindow::onComplianceEvent() {
    qCritical() << "Compliance Event";
}


void
MainWindow::onKeithleyReadyForTrigger() {
    isK236ReadyForTrigger = true;
}


bool
MainWindow::onKeithleyReadyForSweepTrigger() {
    disconnect(pKeithley, SIGNAL(readyForTrigger()),
               this, SLOT(onKeithleyReadyForSweepTrigger()));
    return pKeithley->sendTrigger();
}


void
MainWindow::onNewKeithleyReading(QDateTime dataTime, QString sDataRead) {
    Q_UNUSED(dataTime)
    // Decode readings
    QStringList sMeasures = QStringList(sDataRead.split(",", QString::SkipEmptyParts));
    if(sMeasures.count() < 2) {
        qDebug() << "Measurement Format Error";
        return;
    }
    double currentTemperature = pLakeShore->getTemperature();
    double current, voltage;
    if(configureRvsTDialog.bSourceI) {
        current = sMeasures.at(0).toDouble();
        voltage = sMeasures.at(1).toDouble();
    }
    else {
        current = sMeasures.at(1).toDouble();
        voltage = sMeasures.at(0).toDouble();
    }
    ui->temperatureEdit->setText(QString("%1").arg(currentTemperature));
    ui->currentEdit->setText(QString("%1").arg(current, 10, 'g', 4, ' '));
    ui->voltageEdit->setText(QString("%1").arg(voltage, 10, 'g', 4, ' '));

    if(!bRunning)
        return;

    pOutputFile->write(QString("%1 %2 %3")
                       .arg(currentTemperature, 12, 'g', 6, ' ')
                       .arg(voltage, 12, 'g', 6, ' ')
                       .arg(current, 12, 'g', 6, ' ')
                       .toLocal8Bit());
    pOutputFile->flush();
    double x = 1000.0/currentTemperature;
    double y = current/voltage;
    if(x < xDataMin)
        xDataMin = x;
    if(x > xDataMax)
        xDataMax = x;
    if(y < yDataMin)
        yDataMin = y;
    if(y > yDataMax)
        yDataMax = y;

    pChartMeasurements->axisX()->setRange(xDataMin, xDataMax);
    pChartMeasurements->axisX()->setRange(yDataMin, yDataMax);

    if(currentLampStatus == LAMP_OFF) {
//        pPlotMeasurements->NewPoint(iPlotDark, 1000.0/currentTemperature, current/voltage);
//        pPlotMeasurements->UpdatePlot();
        pDarkMeasurements->append(x, y);
        currentLampStatus = LAMP_ON;
        switchLampOn();
    }
    else {
//        pPlotMeasurements->NewPoint(iPlotPhoto, 1000.0/currentTemperature, current/voltage);
//        pPlotMeasurements->UpdatePlot();
        pPhotoMeasurements->append(x, y);
        currentLampStatus = LAMP_OFF;
        pOutputFile->write("\n");
        switchLampOff();
    }
}


void
MainWindow::onKeithleySweepDone(QDateTime dataTime, QString sData) {
    Q_UNUSED(dataTime)
    disconnect(pKeithley, SIGNAL(sweepDone(QDateTime,QString)), this, 0);
    ui->statusBar->showMessage("Sweep Done: Decoding readings...Please wait");
    QStringList sMeasures = QStringList(sData.split(",", QString::SkipEmptyParts));
    if(sMeasures.count() < 2) {
        qCritical() << "no Sweep Values ";
        return;
    }
    ui->statusBar->showMessage("Sweep Done: Updating Plot...Please wait");
    double current, voltage;
    for(int i=0; i<sMeasures.count(); i+=2) {
        if(presentMeasure == IvsVSourceI) {
            current = sMeasures.at(i).toDouble();
            voltage = sMeasures.at(i+1).toDouble();
        }
        else {
            voltage = sMeasures.at(i).toDouble();
            current = sMeasures.at(i+1).toDouble();
        }
        pOutputFile->write(QString("%1 %2 %3\n")
                           .arg(voltage, 12, 'g', 6, ' ')
                           .arg(current, 12, 'g', 6, ' ')
                           .arg(setPointT, 12, 'g', 6, ' ')
                           .toLocal8Bit());
        pPlotMeasurements->NewPoint(1, voltage, current);
    }
    pPlotMeasurements->UpdatePlot();
    pOutputFile->flush();
    if(configureIvsVDialog.bUseThermostat) {
        setPointT += configureIvsVDialog.dTStep;
        qDebug() << QString("New Set Point: %1").arg(setPointT);
        if(setPointT > configureIvsVDialog.dTStop) {
            stopIvsV();
            ui->statusBar->showMessage("Measure Done");
            return;
        }
        isK236ReadyForTrigger = false;
        connect(pKeithley, SIGNAL(complianceEvent()),
                this, SLOT(onComplianceEvent()));
        connect(pKeithley, SIGNAL(readyForTrigger()),
                this, SLOT(onKeithleyReadyForSweepTrigger()));
        connect(&waitingTStartTimer, SIGNAL(timeout()),
                this, SLOT(onTimeToCheckT()));
        waitingTStartTime = QDateTime::currentDateTime();
        // Start the reaching of the Next Temperature
        waitingTStartTimer.start(5000);
        // Configure Thermostat
        pLakeShore->setTemperature(setPointT);
        pLakeShore->switchPowerOn(3);
        ui->statusBar->showMessage(QString("%1 Waiting Next T[%2K]")
                                   .arg(waitingTStartTime.toString())
                                   .arg(setPointT));
    }
    else {
        stopIvsV();
        ui->statusBar->showMessage("Measure Done");
    }
}


void
MainWindow::onIForwardSweepDone(QDateTime dataTime, QString sData) {
    Q_UNUSED(dataTime)
//    qDebug() << "Reverse Direction Handling";
    ui->statusBar->showMessage("Reverse Direction: Sweeping...Please Wait");
    disconnect(pKeithley, SIGNAL(sweepDone(QDateTime,QString)), this, 0);
    QStringList sMeasures = QStringList(sData.split(",", QString::SkipEmptyParts));
    if(sMeasures.count() < 2) {
        qCritical() << "No Sweep Values ";
        return;
    }
    double current, voltage;
    for(int i=0; i<sMeasures.count(); i+=2) {
        if(presentMeasure == IvsVSourceI) {
            current = sMeasures.at(i).toDouble();
            voltage = sMeasures.at(i+1).toDouble();
        }
        else {
            voltage = sMeasures.at(i).toDouble();
            current = sMeasures.at(i+1).toDouble();
        }
        pOutputFile->write(QString("%1 %2 %3\n")
                           .arg(voltage, 12, 'g', 6, ' ')
                           .arg(current, 12, 'g', 6, ' ')
                           .arg(setPointT, 12, 'g', 6, ' ')
                           .toLocal8Bit());
        pPlotMeasurements->NewPoint(1, voltage, current);
    }
    pPlotMeasurements->UpdatePlot();
    pOutputFile->flush();
    double dVStart;
    double dVStop;
    if(junctionDirection > 0) {// Forward junction
        dVStart = configureIvsVDialog.dVStart;
        dVStop = 0.0;
    }
    else {// Reverse Junction
        dVStart = 0.0;
        dVStop = configureIvsVDialog.dVStop;
    }
    int nSweepPoints = configureIvsVDialog.iNSweepPoints;
    double dVStep = qAbs(dVStop - dVStart) / double(nSweepPoints);
    double dDelayms = double(configureIvsVDialog.iWaitTime);
    double dCompliance = qMax(qAbs(configureIvsVDialog.dIStart),
                              qAbs(configureIvsVDialog.dIStop));
    presentMeasure = IvsVSourceV;
    connect(pKeithley, SIGNAL(readyForTrigger()),
            this, SLOT(onKeithleyReadyForSweepTrigger()));
    connect(pKeithley, SIGNAL(sweepDone(QDateTime,QString)),
            this, SLOT(onKeithleySweepDone(QDateTime,QString)));
    pKeithley->initVSweep(dVStart, dVStop, dVStep, dDelayms, dCompliance);
}


void
MainWindow::onVReverseSweepDone(QDateTime dataTime, QString sData) {
    Q_UNUSED(dataTime)
    qDebug() << "Forward Direction Handling";
    ui->statusBar->showMessage("Forward Direction: Sweeping...Please Wait");
    disconnect(pKeithley, SIGNAL(sweepDone(QDateTime,QString)), this, 0);
    QStringList sMeasures = QStringList(sData.split(",", QString::SkipEmptyParts));
    if(sMeasures.count() < 2) {
        qCritical() << "No Sweep Values ";
        return;
    }
    double current, voltage;
    for(int i=0; i<sMeasures.count(); i+=2) {
        if(presentMeasure == IvsVSourceI) {
            current = sMeasures.at(i).toDouble();
            voltage = sMeasures.at(i+1).toDouble();
        }
        else {
            voltage = sMeasures.at(i).toDouble();
            current = sMeasures.at(i+1).toDouble();
        }
        pOutputFile->write(QString("%1 %2 %3\n")
                           .arg(voltage, 12, 'g', 6, ' ')
                           .arg(current, 12, 'g', 6, ' ')
                           .arg(setPointT, 12, 'g', 6, ' ')
                           .toLocal8Bit());
        pPlotMeasurements->NewPoint(1, voltage, current);
    }
    pPlotMeasurements->UpdatePlot();
    pOutputFile->flush();
    double dIStart = configureIvsVDialog.dIStart;
    double dIStop = 0.0;
    int nSweepPoints = configureIvsVDialog.iNSweepPoints;
    double dIStep = qAbs(dIStop - dIStart) / double(nSweepPoints);
    double dDelayms = double(configureIvsVDialog.iWaitTime);
    double dCompliance = qMax(qAbs(configureIvsVDialog.dVStart),
                              qAbs(configureIvsVDialog.dVStop));
    presentMeasure = IvsVSourceI;
    connect(pKeithley, SIGNAL(readyForTrigger()),
            this, SLOT(onKeithleyReadyForSweepTrigger()));
    connect(pKeithley, SIGNAL(sweepDone(QDateTime,QString)),
            this, SLOT(onKeithleySweepDone(QDateTime,QString)));
    pKeithley->initISweep(dIStart, dIStop, dIStep, dDelayms, dCompliance);
}


bool
MainWindow::getNewMeasure() {
    if(!isK236ReadyForTrigger)
        return false;
    isK236ReadyForTrigger = false;
    return pKeithley->sendTrigger();
}
