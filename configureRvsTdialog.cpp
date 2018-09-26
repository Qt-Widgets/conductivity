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
#include "ui_configureRvsTdialog.h"


#include <QSettings>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>


ConfigureRvsTDialog::ConfigureRvsTDialog(QWidget *parent)
    : QDialog(parent)
    , sBaseDir(QDir::homePath())
    , sOutFileName("conductivity.dat")
    , currentMin(-1.0e-2)
    , currentMax(1.0e-2)
    , voltageMin(-110.0)
    , voltageMax(110.0)
    , intervalMin(0.1)
    , intervalMax(10.0)
    , wavelengthMin(200.0)
    , wavelengthMax(1000.0)
    , temperatureMin(0.0)
    , temperatureMax(450.0)
    , TRateMin(0.01)
    , TRateMax(10.0)
    , reachingTTimeMin(0)// In minutes
    , reachingTTimeMax(30)// In minutes
    , stabilizingTTimeMin(0)// In minutes
    , stabilizingTTimeMax(15)// In minutes
    , ui(new Ui::ConfigureRvsTDialog)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);

    iGratingNumber = 1;

    ui->setupUi(this);
    sHeader = QString("Enter values in range [%1 : %2]");

    sNormalStyle = ui->testValueEdit->styleSheet();
    sErrorStyle  = "QLineEdit { color: rgb(255, 255, 255); background: rgb(255, 0, 0); selection-background-color: rgb(128, 128, 255); }";

    restoreSettings();
    setToolTips();
    setCaptions(bSourceI);

    // Measurement parameters
    ui->testValueEdit->setText(QString("%1").arg(dSourceValue, 0, 'g', 4));
    if(!isSourceValueValid()) {
        qDebug() << QString("Invalid Source Value %1").arg(dSourceValue);
        dSourceValue = 0.0;
        ui->testValueEdit->setText(QString("%1").arg(dSourceValue, 0, 'g', 4));
    }
    ui->complianceValueEdit->setText(QString("%1").arg(dCompliance, 0, 'g', 4));
    if(!isComplianceValueValid()) {
        qDebug() << QString("Invalid Compliance Value %1").arg(dCompliance);
        dCompliance = 0.0;
        ui->complianceValueEdit->setText(QString("%1").arg(dCompliance, 0, 'g', 4));
    }
    if(!isIntervalValid(dInterval)) {
        qDebug() << QString("Invalid Measurement Interval %1").arg(dInterval);
        dInterval = intervalMin;
    }
    ui->measureIntervalEdit->setText(QString("%1").arg(dInterval, 0, 'f', 2));

    // Monochromator parameters
    if(iGratingNumber == 1) {
        ui->radioButtonGrating1->setChecked(true);
        ui->radioButtonGrating2->setChecked(false);
    }
    else {
        iGratingNumber = 2;
        ui->radioButtonGrating1->setChecked(false);
        ui->radioButtonGrating2->setChecked(true);
    }
    if(!isWavelengthValid(dWavelength)) {
        qDebug() << QString("Invalid Wavelength value %1").arg(dWavelength);
        dWavelength = wavelengthMax;
    }
    ui->WavelengthEdit->setText(QString("%1").arg(dWavelength, 0, 'f', 2));

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

    // Timing parameters
    if(!isReachingTimeValid(iReachingTime)) {
        iReachingTime = reachingTTimeMin;
    }
    ui->MaxTimeToTStartEdit->setText(QString("%1").arg(iReachingTime));
    if(!isStabilizingTimeValid(iStabilizingTime)) {
        iStabilizingTime = stabilizingTTimeMin;
    }
    ui->TimeForStabilizingTStartEdit->setText(QString("%1").arg(iStabilizingTime));

    // Sample information
    ui->sampleInformationEdit->setPlainText(sSampleInfo);

    // Output Path
    ui->outPathEdit->setText(sBaseDir);

    // Output File Name
    ui->outFileEdit->setText(sOutFileName);
}


ConfigureRvsTDialog::~ConfigureRvsTDialog() {
    //  qDebug() << "ConfigureRvsTDialog::~ConfigureRvsTDialog()";
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
    //  qDebug() << "ConfigureRvsTDialog::restoreSettings()";
    restoreGeometry(settings.value("ConfigureRvsTDialogGeometry").toByteArray());

    bSourceI         = settings.value("ConfigureRvsTSourceI", true).toBool();
    dSourceValue     = settings.value("ConfigureRvsTSourceValue", 0.0).toDouble();
    dCompliance      = settings.value("ConfigureRvsTCompliance", 0.0).toDouble();
    dInterval        = settings.value("ConfigureRvsTMeasureInterval", 0.1).toDouble();
    dWavelength      = settings.value("ConfigureRvsTWavelength", wavelengthMax).toDouble();
    iGratingNumber   = settings.value("ConfigureRvsTGrating", 1).toInt();
    dTempStart       = settings.value("ConfigureRvsTTempertureStart", 300.0).toDouble();
    dTempEnd         = settings.value("ConfigureRvsTTempertureEnd", 300.0).toDouble();
    dTRate           = settings.value("ConfigureRvsTTRate", 1.0).toDouble();

    sSampleInfo      = settings.value("ConfigureRvsTSampleInfo", "").toString();
    sBaseDir         = settings.value("ConfigureRvsTBaseDir", sBaseDir).toString();
    sOutFileName     = settings.value("ConfigureRvsTOutFileName", sOutFileName).toString();
    iReachingTime    = settings.value("ConfigureRvsTReachingTime", reachingTTimeMin).toInt();
    iStabilizingTime = settings.value("ConfigureRvsTStabilizingTime", stabilizingTTimeMin).toInt();
}


void
ConfigureRvsTDialog::saveSettings() {
    QSettings settings;
    //  qDebug() << "ConfigureRvsTDialog::saveSettings()";
    settings.setValue("ConfigureRvsTSourceI", bSourceI);
    settings.setValue("ConfigureRvsTSourceValue", dSourceValue);
    settings.setValue("ConfigureRvsTCompliance", dCompliance);
    settings.setValue("ConfigureRvsTMeasureInterval", dInterval);
    settings.setValue("ConfigureRvsTWavelength", dWavelength);
    settings.setValue("ConfigureRvsTGrating", iGratingNumber);
    settings.setValue("ConfigureRvsTTempertureStart", dTempStart);
    settings.setValue("ConfigureRvsTTempertureEnd", dTempEnd);
    settings.setValue("ConfigureRvsTTRate", dTRate);
    settings.setValue("ConfigureRvsTReachingTime", iReachingTime);
    settings.setValue("ConfigureRvsTStabilizingTime", iStabilizingTime);
    sSampleInfo = ui->sampleInformationEdit->toPlainText();
    settings.setValue("ConfigureRvsTSampleInfo", sSampleInfo);
    settings.setValue("ConfigureRvsTBaseDir", sBaseDir);
    settings.setValue("ConfigureRvsTOutFileName", sOutFileName);
}


void
ConfigureRvsTDialog::setToolTips() {
    if(bSourceI) {
        ui->testValueEdit->setToolTip(sHeader.arg(currentMin).arg(currentMax));
        ui->complianceValueEdit->setToolTip(sHeader.arg(voltageMin).arg(voltageMax));
    }
    else {
        ui->testValueEdit->setToolTip(sHeader.arg(voltageMin).arg(voltageMax));
        ui->complianceValueEdit->setToolTip(sHeader.arg(currentMin).arg(currentMax));
    }
    ui->measureIntervalEdit->setToolTip(sHeader.arg(intervalMin).arg(intervalMax));
    ui->WavelengthEdit->setToolTip(sHeader.arg(wavelengthMin).arg(wavelengthMax));
    ui->TStartEdit->setToolTip(sHeader.arg(temperatureMin).arg(temperatureMax));
    ui->TEndEdit->setToolTip(sHeader.arg(temperatureMin).arg(temperatureMax));
    ui->TRateEdit->setToolTip(sHeader.arg(TRateMin).arg(TRateMax));
    ui->MaxTimeToTStartEdit->setToolTip(sHeader.arg(reachingTTimeMin).arg(reachingTTimeMax));
    ui->TimeForStabilizingTStartEdit->setToolTip(QString(sHeader.arg(stabilizingTTimeMin).arg(stabilizingTTimeMax)));
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
        ui->labelComplianceUnits->setText("V");
    }
    else {
        ui->labelSourceVal->setText("Voltage");
        ui->labelUnits->setText("V");
        ui->labelComplianceUnits->setText("A");
    }
}


void
ConfigureRvsTDialog::on_radioButtonSourceI_clicked() {
    bSourceI = true;
    ui->testValueEdit->setToolTip(sHeader.arg(currentMin).arg(currentMax));
    ui->complianceValueEdit->setToolTip(sHeader.arg(voltageMin).arg(voltageMax));
    setCaptions(bSourceI);
    if(!isSourceValueValid()) {
        dSourceValue = 0.0;
        ui->testValueEdit->setText(QString("%1").arg(dSourceValue, 0, 'g', 2));
    }
    if(!isComplianceValueValid()) {
        dCompliance = 0.0;
        ui->complianceValueEdit->setText(QString("%1").arg(dCompliance, 0, 'g', 2));
    }
    ui->testValueEdit->setStyleSheet(sNormalStyle);
    ui->complianceValueEdit->setStyleSheet(sNormalStyle);
}


void
ConfigureRvsTDialog::on_radioButtonSourceV_clicked() {
    bSourceI = false;
    ui->testValueEdit->setToolTip(sHeader.arg(voltageMin).arg(voltageMax));
    ui->complianceValueEdit->setToolTip(sHeader.arg(currentMin).arg(currentMax));
    setCaptions(bSourceI);
    if(!isSourceValueValid()) {
        dSourceValue = 0.0;
        ui->testValueEdit->setText(QString("%1").arg(dSourceValue, 0, 'g', 2));
    }
    if(!isComplianceValueValid()) {
        dCompliance = 0.0;
        ui->complianceValueEdit->setText(QString("%1").arg(dCompliance, 0, 'g', 2));
    }
    ui->testValueEdit->setStyleSheet(sNormalStyle);
    ui->complianceValueEdit->setStyleSheet(sNormalStyle);
}


bool
ConfigureRvsTDialog::isSourceValueValid() {
    bool ok;
    double tmp = ui->testValueEdit->text().toDouble(&ok);
    if(!ok) {
        return false;
    }
    if(bSourceI) {
        if((tmp >= currentMin) &&(tmp <= currentMax)) {
            return true;
        }
        else {
            return false;
        }
    }
    else
        if((tmp >= voltageMin) && (tmp <= voltageMax)) {
            return true;
        }
        else {
            return false;
        }
}


bool
ConfigureRvsTDialog::isComplianceValueValid() {
    bool ok;
    double tmp = ui->complianceValueEdit->text().toDouble(&ok);
    if(!ok) {
        return false;
    }
    if(bSourceI) {
        if((tmp >= voltageMin) &&(tmp <= voltageMax)) {
            return true;
        }
        else {
            return false;
        }
    }
    else
        if((tmp >= currentMin) && (tmp <= currentMax)) {
            return true;
        }
        else {
            return false;
        }
}


bool
ConfigureRvsTDialog::isIntervalValid(double interval) {
    return (interval >= intervalMin) && (interval <= intervalMax);
}


bool
ConfigureRvsTDialog::isTemperatureValueValid(double dTemperature) {
    return (dTemperature >= temperatureMin) && (dTemperature <= temperatureMax);
}


bool
ConfigureRvsTDialog::isTRateValid(double dTRate) {
    return (dTRate >= TRateMin) && (dTRate <= TRateMax);
}


bool
ConfigureRvsTDialog::isReachingTimeValid(int iReachingTime) {
    return (iReachingTime >= reachingTTimeMin) &&
            (iReachingTime <= reachingTTimeMax);
}


bool
ConfigureRvsTDialog::isStabilizingTimeValid(int iStabilizingTime) {
    return (iStabilizingTime >= stabilizingTTimeMin) &&
            (iStabilizingTime <= stabilizingTTimeMax);
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
ConfigureRvsTDialog::on_complianceValueEdit_textChanged(const QString &arg1) {
    Q_UNUSED(arg1)
    if(isComplianceValueValid()) {
        dCompliance = ui->complianceValueEdit->text().toDouble();
        ui->complianceValueEdit->setStyleSheet(sNormalStyle);
    }
    else {
        ui->complianceValueEdit->setStyleSheet(sErrorStyle);
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
    if(isTemperatureValueValid(arg1.toDouble())) {
        dTempEnd = arg1.toDouble();
        ui->TEndEdit->setStyleSheet(sNormalStyle);
    }
    else {
        ui->TEndEdit->setStyleSheet(sErrorStyle);
    }
}


void
ConfigureRvsTDialog::on_MaxTimeToTStartEdit_textChanged(const QString &arg1) {
    if(isReachingTimeValid(arg1.toInt())) {
        iReachingTime = arg1.toInt();
        ui->MaxTimeToTStartEdit->setStyleSheet(sNormalStyle);
    }
    else {
        ui->MaxTimeToTStartEdit->setStyleSheet(sErrorStyle);
    }
}


void
ConfigureRvsTDialog::on_TimeForStabilizingTStartEdit_textChanged(const QString &arg1) {
    if(isStabilizingTimeValid(arg1.toInt())) {
        iStabilizingTime = arg1.toInt();
        ui->TimeForStabilizingTStartEdit->setStyleSheet(sNormalStyle);
    }
    else {
        ui->TimeForStabilizingTStartEdit->setStyleSheet(sErrorStyle);
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
    if(sOutFileName == QString()) {
        QMessageBox::information(
                    this,
                    QString("Empty Output Filename"),
                    QString("Please enter a Valid Output File Name"));
        ui->outFileEdit->setFocus();
        return;
    }
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
    saveSettings();
    accept();
}


void
ConfigureRvsTDialog::on_cancelButton_clicked() {
    reject();
}


void
ConfigureRvsTDialog::on_measureIntervalEdit_textChanged(const QString &arg1) {
    if(isIntervalValid(arg1.toDouble())){
        dInterval = arg1.toDouble();
        ui->measureIntervalEdit->setStyleSheet(sNormalStyle);
    }
    else {
        ui->measureIntervalEdit->setStyleSheet(sErrorStyle);
    }
}


void
ConfigureRvsTDialog::on_radioButtonGrating1_clicked() {
    iGratingNumber = 1;
}


void
ConfigureRvsTDialog::on_radioButtonGrating2_clicked() {
    iGratingNumber = 2;
}


void
ConfigureRvsTDialog::on_WavelengthEdit_textChanged(const QString &arg1) {
    if(isWavelengthValid(arg1.toDouble())) {
        dWavelength = arg1.toDouble();
        ui->WavelengthEdit->setStyleSheet(sNormalStyle);
    }
    else {
        ui->WavelengthEdit->setStyleSheet(sErrorStyle);
    }
}


bool
ConfigureRvsTDialog::isWavelengthValid(double wavelength) {
    return (wavelength >= wavelengthMin) && (wavelength <= wavelengthMax);
}

