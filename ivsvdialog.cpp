#include "ivsvdialog.h"

#include <QTabWidget>
#include <QVBoxLayout>


IvsVDialog::IvsVDialog(QWidget *parent)
    : QDialog(parent)
{
    setAttribute(Qt::WA_AlwaysShowToolTips);

    iSourceIndex = tabWidget.addTab(&TabK236,  tr("K236"));
    iThermIndex  = tabWidget.addTab(&TabLS330, tr("LS330"));
    iMonoIndex   = tabWidget.addTab(&TabCS130, tr("CS130"));
    iFileIndex   = tabWidget.addTab(&TabFile,  tr("Out File"));

    buttonBox.addButton(QString("Ok"), QDialogButtonBox::ButtonRole::AcceptRole);
    buttonBox.addButton(QString("Cancel"), QDialogButtonBox::ButtonRole::RejectRole);
//            = new QDialogButtonBox(QDialogButtonBox::Ok |
//                                     QDialogButtonBox::Cancel);

    connectSignals();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(&tabWidget);
    mainLayout->addWidget(&buttonBox);
    setLayout(mainLayout);
    setWindowTitle("I versus V");
    setToolTips();
}


void
IvsVDialog::setToolTips() {
    tabWidget.setTabToolTip(iSourceIndex, QString("Source-Measure Unit configuration"));
    tabWidget.setTabToolTip(iThermIndex, QString("Thermostat configuration"));
    tabWidget.setTabToolTip(iMonoIndex, QString("Monochromator configuration"));
    tabWidget.setTabToolTip(iFileIndex, QString("Output File configuration"));
}


void
IvsVDialog::connectSignals() {
    connect(&buttonBox, SIGNAL(accepted()),
            this, SLOT(onOk()));
    connect(&buttonBox, SIGNAL(rejected()),
            this, SLOT(onCancel()));
}


void
IvsVDialog::onCancel() {
    TabK236.restoreSettings();
    TabLS330.restoreSettings();
    TabCS130.restoreSettings();
    TabFile.restoreSettings();
    reject();
}



void
IvsVDialog::onOk() {
    if(TabFile.checkFileName()) {
        TabK236.saveSettings();
        TabLS330.saveSettings();
        TabCS130.saveSettings();
        TabFile.saveSettings();
        accept();
    }
    tabWidget.setCurrentIndex(iFileIndex);
}



