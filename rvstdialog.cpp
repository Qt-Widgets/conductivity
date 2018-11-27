#include "rvstdialog.h"

#include <QVBoxLayout>

RvsTDialog::RvsTDialog(QWidget *parent)
    : QDialog(parent)
{
    pTabK236  = new K236Tab(2, this);//1 => IvsV configuration
    pTabLS330 = new LS330Tab(2, this);
    pTabCS130 = new CS130Tab(2, this);
    pTabFile  = new FileTab(2, this);

    pTabWidget = new QTabWidget();

    iSourceIndex = pTabWidget->addTab(pTabK236,  tr("K236"));
    iThermIndex  = pTabWidget->addTab(pTabLS330, tr("LS330"));
    iMonoIndex   = pTabWidget->addTab(pTabCS130, tr("CS130"));
    iFileIndex   = pTabWidget->addTab(pTabFile,  tr("Out File"));

    pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                      QDialogButtonBox::Cancel);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(pTabWidget);
    mainLayout->addWidget(pButtonBox);
    setLayout(mainLayout);

    setWindowTitle("R versus T");
    connectSignals();
    setToolTips();
}


void
RvsTDialog::setToolTips() {
    pTabWidget->setTabToolTip(iSourceIndex, QString("Source-Measure Unit configuration"));
    pTabWidget->setTabToolTip(iThermIndex, QString("Thermostat configuration"));
    pTabWidget->setTabToolTip(iMonoIndex, QString("Monochromator configuration"));
    pTabWidget->setTabToolTip(iFileIndex, QString("Output File configuration"));
}


void
RvsTDialog::connectSignals() {
    connect(pButtonBox, SIGNAL(accepted()),
            this, SLOT(onOk()));
    connect(pButtonBox, SIGNAL(rejected()),
            this, SLOT(onCancel()));
}


void
RvsTDialog::onCancel() {
    pTabK236->restoreSettings();
    pTabLS330->restoreSettings();
    pTabCS130->restoreSettings();
    pTabFile->restoreSettings();
    reject();
}



void
RvsTDialog::onOk() {
    if(pTabFile->checkFileName()) {
        pTabK236->saveSettings();
        pTabLS330->saveSettings();
        pTabCS130->saveSettings();
        pTabFile->saveSettings();
        accept();
    }
    pTabWidget->setCurrentIndex(iFileIndex);
}



