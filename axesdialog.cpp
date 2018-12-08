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
#include "axesdialog.h"
#include "ui_axesdialog.h"

AxesDialog::AxesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AxesDialog)
{
    ui->setupUi(this);
}


AxesDialog::~AxesDialog() {
    delete ui;
}


void
AxesDialog::initDialog(AxisLimits AxisLimits) {
    newLimits = AxisLimits;
    ui->editXMin->setText(QString::number(AxisLimits.XMin, 'g', 4));
    ui->editXMax->setText(QString::number(AxisLimits.XMax, 'g', 4));
    ui->editYMin->setText(QString::number(AxisLimits.YMin, 'g', 4));
    ui->editYMax->setText(QString::number(AxisLimits.YMax, 'g', 4));
    ui->autoX->setChecked(AxisLimits.AutoX);
    ui->autoY->setChecked(AxisLimits.AutoY);
    ui->LogX->setChecked(AxisLimits.LogX);
    ui->LogY->setChecked(AxisLimits.LogY);
}


void
AxesDialog::on_buttonBox_accepted() {
    newLimits.XMin =  ui->editXMin->text().toDouble();
    newLimits.XMax =  ui->editXMax->text().toDouble();
    newLimits.YMin =  ui->editYMin->text().toDouble();
    newLimits.YMax =  ui->editYMax->text().toDouble();
    newLimits.AutoX=  ui->autoX->isChecked();
    newLimits.AutoY=  ui->autoY->isChecked();
    newLimits.LogX =  ui->LogX->isChecked();
    newLimits.LogY =  ui->LogY->isChecked();
    accept();
}


void
AxesDialog::on_buttonBox_rejected() {
    accept();
}
