#include "configuredialog.h"
#include "ui_configuredialog.h"

#include <QSettings>
#include <QDebug>


ConfigureDialog::ConfigureDialog(QWidget *parent)
  : QDialog(parent)
  , currentMin(-1.0e-3)
  , currentMax(1.0e-3)
  , voltageMin(-10.0)
  , voltageMax(10.0)
  , temperatureMin(0.0)
  , temperatureMax(450.0)
  , sweepTimeMin(180)// Three minutes
  , sweepTimeMax(24*60*60)// A day !
  , ui(new Ui::ConfigureDialog)
{
  ui->setupUi(this);

  sNormalStyle = ui->testValueEdit->styleSheet();
  sErrorStyle  = "QLineEdit { color: rgb(255, 255, 255); background: rgb(255, 0, 0); selection-background-color: rgb(128, 128, 255); }";

  QSettings settings;
  restoreGeometry(settings.value("configureDialogGeometry").toByteArray());

  // Measurement parameters
  bSourceI = settings.value("configureSourceI", true).toBool();
  setCaptions(bSourceI);
  dSourceValue = settings.value("configureSourceValue", 0.0).toDouble();
  ui->testValueEdit->setText(QString("%1").arg(dSourceValue, 0, 'g', 2));
  if(!isSourceValueValid()) {
    dSourceValue = 0.0;
    ui->testValueEdit->setText(QString("%1").arg(dSourceValue, 0, 'g', 2));
  }
  // Temperature parameters
  dTempStart = settings.value("configureTempertureStart", 300.0).toDouble();
  if(!isTemperatureValueValid(dTempStart)) {
    dTempStart = 300.0;
  }
  ui->TStartEdit->setText(QString("%1").arg(dTempStart, 0, 'f', 2));
  dTempEnd = settings.value("configureTempertureEnd", 300.0).toDouble();
  if(!isTemperatureValueValid(dTempEnd)) {
    dTempEnd = 300.0;
  }
  ui->TEndEdit->setText(QString("%1").arg(dTempEnd, 0, 'f', 2));


  iSweepTime = settings.value("configureSweepTime", 300).toInt();
  if(iSweepTime < 300) {
    iSweepTime = 300.0;
  }
  ui->SweepTimeEdit->setText(QString("%1").arg(iSweepTime));
}


ConfigureDialog::~ConfigureDialog() {
  delete ui;
}

void
ConfigureDialog::closeEvent(QCloseEvent *event) {
  Q_UNUSED(event)
  QSettings settings;
  bSourceI = ui->radioButtonSourceI->isChecked();
  settings.setValue("configureSourceI", bSourceI);
  settings.setValue("configureSourceValue", dSourceValue);
}


void
ConfigureDialog::setCaptions(bool bSourceI) {
  ui->radioButtonSourceI->setChecked(bSourceI);
  ui->radioButtonSourceV->setChecked(!bSourceI);
  if(bSourceI) {
    ui->labelSourceVal->setText("Current");
    ui->labelUnits->setText("A");
  }
  else {
    ui->labelSourceVal->setText("Voltage");
    ui->labelUnits->setText("V");
  }
}


void
ConfigureDialog::on_radioButtonSourceI_clicked() {
  bSourceI = ui->radioButtonSourceI->isChecked();
  setCaptions(bSourceI);
  if(!isSourceValueValid()) {
    dSourceValue = 0.0;
    ui->testValueEdit->setText(QString("%1").arg(dSourceValue, 0, 'g', 2));
  }
  ui->testValueEdit->setStyleSheet(sNormalStyle);
}

void
ConfigureDialog::on_radioButtonSourceV_clicked() {
  bSourceI = ui->radioButtonSourceI->isChecked();
  setCaptions(bSourceI);
  if(!isSourceValueValid()) {
    dSourceValue = 0.0;
    ui->testValueEdit->setText(QString("%1").arg(dSourceValue, 0, 'g', 2));
  }
  ui->testValueEdit->setStyleSheet(sNormalStyle);
}


bool
ConfigureDialog::isSourceValueValid() {
  bool ok;
  double tmp = ui->testValueEdit->text().toDouble(&ok);
  if(!ok) {
    return false;
  }
  if(bSourceI)
    if((tmp >= currentMin) &&(tmp <= currentMax))
      return true;
    else {
      return false;
    }
  else
    if((tmp >= voltageMin) && (tmp <= voltageMax))
      return true;
    else {
      return false;
    }
}


bool
ConfigureDialog::isTemperatureValueValid(double dTemperature) {
  return (dTemperature >= temperatureMin) && (dTemperature >= temperatureMax);
}


void
ConfigureDialog::on_testValueEdit_textChanged(const QString &arg1) {
  Q_UNUSED(arg1)
  if(isSourceValueValid()) {
    dSourceValue = ui->testValueEdit->text().toDouble();
    ui->testValueEdit->setStyleSheet(sNormalStyle);
  }
  else {
    ui->testValueEdit->setStyleSheet(sErrorStyle);
  }
}
