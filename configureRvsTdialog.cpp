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
#include "configureRvsTdialog.h"
#include "ui_ConfigureRvsTDialog.h"


#include <QSettings>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>


ConfigureRvsTDialog::ConfigureRvsTDialog(QWidget *parent)
  : QDialog(parent)
  , sBaseDir(QDir::homePath())
  , sOutFileName("conductivity.dat")
  , currentMin(-1.0e-3)
  , currentMax(1.0e-3)
  , voltageMin(-10.0)
  , voltageMax(10.0)
  , temperatureMin(0.0)
  , temperatureMax(450.0)
  , TRateMin(0.01)
  , TRateMax(10.0)
  , ui(new Ui::ConfigureRvsTDialog)
{
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
  setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);
  ui->setupUi(this);

  sNormalStyle = ui->testValueEdit->styleSheet();
  sErrorStyle  = "QLineEdit { color: rgb(255, 255, 255); background: rgb(255, 0, 0); selection-background-color: rgb(128, 128, 255); }";

  restoreSettings();
  setToolTips();

  // Measurement parameters
  setCaptions(bSourceI);
  ui->testValueEdit->setText(QString("%1").arg(dSourceValue, 0, 'g', 2));
  if(!isSourceValueValid()) {
    dSourceValue = 0.0;
    ui->testValueEdit->setText(QString("%1").arg(dSourceValue, 0, 'g', 2));
  }

  // Temperature parameters
  if(!isTemperatureValueValid(dTempStart)) {
    dTempStart = 300.0;
  }
  ui->TStartEdit->setText(QString("%1").arg(dTempStart, 0, 'f', 2));
  if(!isTemperatureValueValid(dTempEnd)) {
    dTempEnd = 300.0;
  }
  ui->TEndEdit->setText(QString("%1").arg(dTempEnd, 0, 'f', 2));

  // Sweep Time parameter
  if(!isTRateValid(dTRate)) {
    dTRate = 1.0;
  }
  ui->TRateEdit->setText(QString("%1").arg(dTRate, 0, 'f', 2));

  // Sample information
  ui->sampleInformationEdit->setPlainText(sSampleInfo);

  // Output Path
  ui->outPathEdit->setText(sBaseDir);

  // Output File Name
  ui->outFileEdit->setText(sOutFileName);
}


ConfigureRvsTDialog::~ConfigureRvsTDialog() {
  delete ui;
}


void
ConfigureRvsTDialog::closeEvent(QCloseEvent *event) {
  Q_UNUSED(event)
  saveSettings();
}


void
ConfigureRvsTDialog::restoreSettings() {
  QSettings settings;

  restoreGeometry(settings.value("ConfigureRvsTDialogGeometry").toByteArray());

  bSourceI     = settings.value("ConfigureRvsTSourceI", true).toBool();
  dSourceValue = settings.value("ConfigureRvsTSourceValue", 0.0).toDouble();
  dTempStart   = settings.value("ConfigureRvsTTempertureStart", 300.0).toDouble();
  dTempEnd     = settings.value("ConfigureRvsTTempertureEnd", 300.0).toDouble();
  dTRate       = settings.value("ConfigureRvsTTRate", 1.0).toDouble();
  sSampleInfo  = settings.value("ConfigureRvsTSampleInfo", "").toString();
  sBaseDir     = settings.value("ConfigureRvsTBaseDir", sBaseDir).toString();
  sOutFileName = settings.value("ConfigureRvsTOutFileName", sOutFileName).toString();
}


void
ConfigureRvsTDialog::saveSettings() {
  QSettings settings;

  settings.setValue("ConfigureRvsTSourceI", bSourceI);
  settings.setValue("ConfigureRvsTSourceValue", dSourceValue);
  settings.setValue("ConfigureRvsTTempertureStart", dTempStart);
  settings.setValue("ConfigureRvsTTempertureEnd", dTempEnd);
  settings.setValue("ConfigureRvsTTRate", dTRate);
  sSampleInfo = ui->sampleInformationEdit->toPlainText();
  settings.setValue("ConfigureRvsTSampleInfo", sSampleInfo);
  settings.setValue("ConfigureRvsTBaseDir", sBaseDir);
  settings.setValue("ConfigureRvsTOutFileName", sOutFileName);
}


void
ConfigureRvsTDialog::setToolTips() {
  QString sHeader = QString("Enter values in range [%1 : %2]");
  if(bSourceI)
    ui->testValueEdit->setToolTip(sHeader.arg(currentMin).arg(currentMax));
  else
    ui->testValueEdit->setToolTip(sHeader.arg(voltageMin).arg(voltageMax));

  ui->TStartEdit->setToolTip(sHeader.arg(temperatureMin).arg(temperatureMax));
  ui->TEndEdit->setToolTip(sHeader.arg(temperatureMin).arg(temperatureMax));
  ui->TRateEdit->setToolTip(sHeader.arg(TRateMin).arg(TRateMax));
  ui->sampleInformationEdit->setToolTip(QString("Enter Sample description (multiline)"));
  ui->outPathEdit->setToolTip(QString("Output File Folder"));
  ui->outFileEdit->setToolTip(QString("Enter Output File Name"));
  ui->outFilePathButton->setToolTip((QString("Press to Change Output File Folder")));
  ui->doneButton->setToolTip(QString("Press when Done"));
}


void
ConfigureRvsTDialog::setCaptions(bool bSourceI) {
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
ConfigureRvsTDialog::on_radioButtonSourceI_clicked() {
  bSourceI = true;
  ui->testValueEdit->setToolTip(QString("Enter values in range [%1 : %2]").arg(currentMin).arg(currentMax));
  setCaptions(bSourceI);
  if(!isSourceValueValid()) {
    dSourceValue = 0.0;
    ui->testValueEdit->setText(QString("%1").arg(dSourceValue, 0, 'g', 2));
  }
  ui->testValueEdit->setStyleSheet(sNormalStyle);
}


void
ConfigureRvsTDialog::on_radioButtonSourceV_clicked() {
  bSourceI = false;
  ui->testValueEdit->setToolTip(QString("Enter values in range [%1 : %2]").arg(voltageMin).arg(voltageMax));
  setCaptions(bSourceI);
  if(!isSourceValueValid()) {
    dSourceValue = 0.0;
    ui->testValueEdit->setText(QString("%1").arg(dSourceValue, 0, 'g', 2));
  }
  ui->testValueEdit->setStyleSheet(sNormalStyle);
}


bool
ConfigureRvsTDialog::isSourceValueValid() {
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
ConfigureRvsTDialog::isTemperatureValueValid(double dTemperature) {
  return (dTemperature >= temperatureMin) && (dTemperature <= temperatureMax);
}


bool
ConfigureRvsTDialog::isTRateValid(double dTRate) {
  return (dTRate >= TRateMin) && (dTRate <= TRateMax);
}


void
ConfigureRvsTDialog::on_testValueEdit_textChanged(const QString &arg1) {
  Q_UNUSED(arg1)
  if(isSourceValueValid()) {
    dSourceValue = ui->testValueEdit->text().toDouble();
    ui->testValueEdit->setStyleSheet(sNormalStyle);
  }
  else {
    ui->testValueEdit->setStyleSheet(sErrorStyle);
  }
}


void
ConfigureRvsTDialog::on_TStartEdit_textChanged(const QString &arg1) {
  if(isTemperatureValueValid(arg1.toDouble())){
    dTempStart = arg1.toDouble();
    ui->TStartEdit->setStyleSheet(sNormalStyle);
  }
  else {
    ui->TStartEdit->setStyleSheet(sErrorStyle);
  }
}


void
ConfigureRvsTDialog::on_TEndEdit_textChanged(const QString &arg1) {
  if(isTemperatureValueValid(arg1.toDouble())){
    dTempEnd = arg1.toDouble();
    ui->TEndEdit->setStyleSheet(sNormalStyle);
  }
  else {
    ui->TEndEdit->setStyleSheet(sErrorStyle);
  }
}


void
ConfigureRvsTDialog::on_outFilePathButton_clicked() {
  QFileDialog chooseDirDialog;
  QDir outDir(sBaseDir);
  chooseDirDialog.setFileMode(QFileDialog::DirectoryOnly);
  if(outDir.exists()) {
    chooseDirDialog.setDirectory(outDir);
  }
  else {
    chooseDirDialog.setDirectory(QDir::homePath());
  }
  if(chooseDirDialog.exec() == QDialog::Accepted) {
    sBaseDir = chooseDirDialog.selectedFiles().at(0);
  }
  ui->outPathEdit->setText(sBaseDir);
}


void
ConfigureRvsTDialog::on_TRateEdit_textChanged(const QString &arg1) {
    if(isTRateValid(arg1.toDouble())){
      dTRate = arg1.toDouble();
      ui->TRateEdit->setStyleSheet(sNormalStyle);
    }
    else {
      ui->TRateEdit->setStyleSheet(sErrorStyle);
    }
}


void
ConfigureRvsTDialog::on_doneButton_clicked() {
  sOutFileName = ui->outFileEdit->text();
  if(QDir(sBaseDir).exists(sOutFileName)) {
    int iAnswer = QMessageBox::question(
                    this,
                    QString("File Already exists"),
                    QString("Overwrite %1 ?").arg(sOutFileName),
                    QMessageBox::Yes,
                    QMessageBox::No,
                    QMessageBox::NoButton);
    if(iAnswer == QMessageBox::No) {
      ui->outFileEdit->setFocus();
      return;
    }
  }
  accept();
}


void
ConfigureRvsTDialog::on_cancelButton_clicked() {
  reject();
}
