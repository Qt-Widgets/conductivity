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
#include "configureIvsVdialog.h"
#include "ui_configureIvsVdialog.h"


#include <QSettings>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>


ConfigureIvsVDialog::ConfigureIvsVDialog(QWidget *parent)
  : QDialog(parent)
  , sBaseDir(QDir::homePath())
  , sOutFileName("junction.dat")
  , ui(new Ui::ConfigureIvsVDialog)
{
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
  setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);
  ui->setupUi(this);

  sNormalStyle = ui->testValueEdit->styleSheet();
  sErrorStyle  = "QLineEdit { color: rgb(255, 255, 255); background: rgb(255, 0, 0); selection-background-color: rgb(128, 128, 255); }";

  restoreSettings();
  setToolTips();

}


ConfigureIvsVDialog::~ConfigureIvsVDialog() {
  delete ui;
}


void
ConfigureIvsVDialog::closeEvent(QCloseEvent *event) {
  Q_UNUSED(event)
  saveSettings();
}


void
ConfigureIvsVDialog::restoreSettings() {
  QSettings settings;

  restoreGeometry(settings.value("ConfigureIvsVDialogGeometry").toByteArray());

  sSampleInfo  = settings.value("ConfigureIvsVSampleInfo", "").toString();
  sBaseDir     = settings.value("ConfigureIvsVBaseDir", sBaseDir).toString();
  sOutFileName = settings.value("ConfigureIvsVOutFileName", sOutFileName).toString();
}


void
ConfigureIvsVDialog::saveSettings() {
  QSettings settings;

  sSampleInfo = ui->sampleInformationEdit->toPlainText();
  settings.setValue("ConfigureIvsVSampleInfo", sSampleInfo);
  settings.setValue("ConfigureIvsVBaseDir", sBaseDir);
  settings.setValue("ConfigureIvsVOutFileName", sOutFileName);
}


void
ConfigureIvsVDialog::setToolTips() {
  QString sHeader = QString("Enter values in range [%1 : %2]");
  Q_UNUSED(sHeader)
}


void
ConfigureIvsVDialog::on_doneButton_clicked() {
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
ConfigureIvsVDialog::on_cancelButton_clicked() {
  reject();
}


void
ConfigureIvsVDialog::on_outFilePathButton_clicked() {
}
