#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <NIDAQmx.h>

#include "configuredialog.h"

namespace Ui {
class MainWindow;
}

QT_FORWARD_DECLARE_CLASS(QFile)
QT_FORWARD_DECLARE_CLASS(Keithley236)
QT_FORWARD_DECLARE_CLASS(LakeShore330)
QT_FORWARD_DECLARE_CLASS(StripChart)

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

private slots:
  void on_configureButton_clicked();

  void on_startButton_clicked();

private:
  Ui::MainWindow *ui;

private:
  QFile* pOutputFile;
  Keithley236* pKeithley;
  LakeShore330* pLakeShore;
  ConfigureDialog configureDialog;

  int GpibBoardID;
  int32       error;
  TaskHandle  DOTaskHandle;// Digital Output
  bool bStartDaq;
  StripChart *pPlotMeasurements;
  StripChart *pPlotTemperature;
  QString sMeasurementPlotLabel;
  QString sTemperaturePlotLabel;
  int nChartPoints;
};

#endif // MAINWINDOW_H
