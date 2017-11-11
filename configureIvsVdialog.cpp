#include "configureIvsVdialog.h"
#include "ui_configureIvsVdialog.h"

ConfigureIvsVDialog::ConfigureIvsVDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ConfigureIvsVDialog)
{
  ui->setupUi(this);
}


ConfigureIvsVDialog::~ConfigureIvsVDialog() {
  delete ui;
}


void
ConfigureIvsVDialog::on_cancelButton_clicked() {
  reject();
}


void
ConfigureIvsVDialog::on_doneButton_clicked() {
  accept();
}
