#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

QT_FORWARD_DECLARE_CLASS(Keithley236)
QT_FORWARD_DECLARE_CLASS(StripChart)

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

protected:
  void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:
  Ui::MainWindow *ui;

private:
  Keithley236* pKeithley;
  bool bStartDaq;
  StripChart *pPlot1;
//  StripChart *plot2;
//  StripChart *plot3;
  int nChartPoints;
};

#endif // MAINWINDOW_H
