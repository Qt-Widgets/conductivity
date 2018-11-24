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

#include "configureRvsTdialog.h"
#include "configureIvsVdialog.h"
#include "ivsvdialog.h"


namespace Ui {
    class MainWindow;
}


QT_FORWARD_DECLARE_CLASS(QFile)
QT_FORWARD_DECLARE_CLASS(Keithley236)
QT_FORWARD_DECLARE_CLASS(LakeShore330)
QT_FORWARD_DECLARE_CLASS(CornerStone130)
QT_FORWARD_DECLARE_CLASS(Plot2D)


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = Q_NULLPTR);
    ~MainWindow() Q_DECL_OVERRIDE;
    bool checkInstruments();

signals:

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    bool getNewMeasure();
    void initRvsTPlots();
    void stopRvsT();
    void startI_V();
    void stopIvsV();
    void initIvsVPlots();
    bool prepareOutputFile(QString sBaseDir, QString sFileName);
    void switchLampOn();
    void switchLampOff();

private slots:
    void on_startRvsTButton_clicked();
    void on_startIvsVButton_clicked();
    void onTimeToCheckReachedT();
    void onTimeToCheckT();
    void onTimerStabilizeT();
    void onSteadyTReached();
    void onTimeToReadT();
    void onTimeToGetNewMeasure();
    void onComplianceEvent();
    void onClearComplianceEvent();
    void onKeithleyReadyForTrigger();
    void onNewKeithleyReading(QDateTime dataTime, QString sDataRead);
    bool onKeithleyReadyForSweepTrigger();
    void onKeithleySweepDone(QDateTime dataTime, QString sData);
    void onIForwardSweepDone(QDateTime, QString sData);
    void onVReverseSweepDone(QDateTime, QString sData);

    void on_lampButton_clicked();

private:
    Ui::MainWindow *ui;

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
    QString sNormalStyle;
    QString sErrorStyle;
    QString sDarkStyle;
    QString sPhotoStyle;

    QFile *pOutputFile;

    Keithley236    *pKeithley;
    LakeShore330   *pLakeShore;
    CornerStone130 *pCornerStone130;

    Plot2D *pPlotMeasurements;
    Plot2D *pPlotTemperature;

    QDateTime     currentTime;
    QDateTime     waitingTStartTime;
    QDateTime     startReadingTTime;
    QDateTime     startMeasuringTime;
    QDateTime     endMeasureTime;

    QTimer        waitingTStartTimer;
    QTimer        stabilizingTimer;
    QTimer        readingTTimer;
    QTimer        measuringTimer;

    ConfigureRvsTDialog configureRvsTDialog;
//    ConfigureIvsVDialog configureIvsVDialog;
    IvsVDialog          configureIvsVDialog;


    const quint8  LAMP_ON  = 1;
    const quint8  LAMP_OFF = 0;
    const int     iPlotDark = 1;
    const int     iPlotPhoto = 2;

    double        currentTemperature;
    double        setPointT;
    int           iCurrentTPlot;
    int           gpibBoardID;
    quint8        currentLampStatus;
    QString       sMeasurementPlotLabel;
    QString       sTemperaturePlotLabel;
    int           maxPlotPoints;
    volatile bool isK236ReadyForTrigger;
    bool          bRunning;
    int           junctionDirection;
    bool          bUseMonochromator;
    int           gpioHostHandle;
    int           gpioLEDpin;
};

   #endif // MAINWINDOW_H
