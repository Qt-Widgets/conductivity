#include "configuredialog.h"
#include "ui_configuredialog.h"


#include <QSettings>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>

ConfigureDialog::ConfigureDialog(QWidget *parent)
  : QDialog(parent)
  , sBaseDir(QDir::homePath())
  , sOutFileName("conductivity.dat")
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
  if(iSweepTime < 300) {
    iSweepTime = 300.0;
  }
  ui->SweepTimeEdit->setText(QString("%1").arg(iSweepTime));

  // Sample information
  ui->sampleInformationEdit->setPlainText(sSampleInfo);

  // Output Path
  ui->outPathEdit->setText(sBaseDir);

  // Output File Name
  ui->outFileEdit->setText(sOutFileName);
}


ConfigureDialog::~ConfigureDialog() {
  delete ui;
}


void
ConfigureDialog::restoreSettings() {
  QSettings settings;

  restoreGeometry(settings.value("configureDialogGeometry").toByteArray());

  bSourceI     = settings.value("configureSourceI", true).toBool();
  dSourceValue = settings.value("configureSourceValue", 0.0).toDouble();
  dTempStart   = settings.value("configureTempertureStart", 300.0).toDouble();
  dTempEnd     = settings.value("configureTempertureEnd", 300.0).toDouble();
  iSweepTime   = settings.value("configureSweepTime", 300).toInt();
  sSampleInfo  = settings.value("configureSampleInfo", "").toString();
  sBaseDir     = settings.value("configureBaseDir", sBaseDir).toString();
  sOutFileName = settings.value("configureOutFileName", sOutFileName).toString();
}


void
ConfigureDialog::saveSettings() {
  QSettings settings;

  settings.setValue("configureSourceI", bSourceI);
  settings.setValue("configureSourceValue", dSourceValue);
  settings.setValue("configureTempertureStart", dTempStart);
  settings.setValue("configureTempertureEnd", dTempEnd);
  settings.setValue("configureSweepTime", iSweepTime);
  sSampleInfo = ui->sampleInformationEdit->toPlainText();
  settings.setValue("configureSampleInfo", sSampleInfo);
  settings.setValue("configureBaseDir", sBaseDir);
  settings.setValue("configureOutFileName", sOutFileName);
}


void
ConfigureDialog::setToolTips() {
  QString sHeader = QString("Enter values in range [%1 : %2]");
  if(bSourceI)
    ui->testValueEdit->setToolTip(sHeader.arg(currentMin).arg(currentMax));
  else
    ui->testValueEdit->setToolTip(sHeader.arg(voltageMin).arg(voltageMax));

  ui->TStartEdit->setToolTip(sHeader.arg(temperatureMin).arg(temperatureMax));
  ui->TEndEdit->setToolTip(sHeader.arg(temperatureMin).arg(temperatureMax));
  ui->SweepTimeEdit->setToolTip(sHeader.arg(sweepTimeMin).arg(sweepTimeMax));
  ui->sampleInformationEdit->setToolTip(QString("Enter Sample description (multiline)"));
  ui->outPathEdit->setToolTip(QString("Output File Folder"));
  ui->outFileEdit->setToolTip(QString("Enter Output File Name"));
  ui->outFilePathButton->setToolTip((QString("Press to Change Output File Folder")));
}


void
ConfigureDialog::closeEvent(QCloseEvent *event) {
  Q_UNUSED(event)
  saveSettings();
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
ConfigureDialog::on_radioButtonSourceV_clicked() {
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
  return (dTemperature >= temperatureMin) && (dTemperature <= temperatureMax);
}


bool
ConfigureDialog::isSweepTimeValid(int iSweep) {
  return (iSweep >= sweepTimeMin) && (iSweep <= sweepTimeMax);
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


void
ConfigureDialog::on_doneButton_clicked() {
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
  close();
}


void
ConfigureDialog::on_TStartEdit_textChanged(const QString &arg1) {
  if(isTemperatureValueValid(arg1.toDouble())){
    dTempStart = arg1.toDouble();
    ui->TStartEdit->setStyleSheet(sNormalStyle);
  }
  else {
    ui->TStartEdit->setStyleSheet(sErrorStyle);
  }
}


void
ConfigureDialog::on_TEndEdit_textChanged(const QString &arg1) {
  if(isTemperatureValueValid(arg1.toDouble())){
    dTempEnd = arg1.toDouble();
    ui->TEndEdit->setStyleSheet(sNormalStyle);
  }
  else {
    ui->TEndEdit->setStyleSheet(sErrorStyle);
  }
}


void
ConfigureDialog::on_SweepTimeEdit_textChanged(const QString &arg1) {
  if(isSweepTimeValid(arg1.toInt())){
    iSweepTime = arg1.toInt();
    ui->SweepTimeEdit->setStyleSheet(sNormalStyle);
  }
  else {
    ui->SweepTimeEdit->setStyleSheet(sErrorStyle);
  }
}


void
ConfigureDialog::on_outFileEdit_editingFinished() {
}


void
ConfigureDialog::on_outFilePathButton_clicked() {
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
