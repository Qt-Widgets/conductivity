#ifndef AXESDIALOG_H
#define AXESDIALOG_H

#include <QDialog>
#include "AxisLimits.h"

namespace Ui {
class AxesDialog;
}

class AxesDialog : public QDialog
{
  Q_OBJECT

public:
  explicit AxesDialog(QWidget *parent = 0);
  ~AxesDialog();
  void initDialog(CAxisLimits AxisLimits);

private:
  Ui::AxesDialog *ui;

public:
  CAxisLimits newLimits;
private slots:
  void on_buttonBox_accepted();
};

#endif // AXESDIALOG_H
