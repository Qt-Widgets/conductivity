#include "configuredialog.h"

#include <QVBoxLayout>


ConfigureDialog::ConfigureDialog(int iConfiguration, QWidget *parent)
    : QDialog(parent)
    , configurationType(iConfiguration)
{
    pTabK236  = new K236Tab(configurationType, this);
    pTabLS330 = new LS330Tab(configurationType, this);
    pTabCS130 = new CS130Tab(configurationType, this);
    pTabFile  = new FileTab(configurationType, this);

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

    setWindowTitle("I versus V");
    connectSignals();
    setToolTips();
}


void
ConfigureDialog::setToolTips() {
    pTabWidget->setTabToolTip(iSourceIndex, QString("Source-Measure Unit configuration"));
    pTabWidget->setTabToolTip(iThermIndex, QString("Thermostat configuration"));
    pTabWidget->setTabToolTip(iMonoIndex, QString("Monochromator configuration"));
    pTabWidget->setTabToolTip(iFileIndex, QString("Output File configuration"));
}


void
ConfigureDialog::connectSignals() {
    connect(pButtonBox, SIGNAL(accepted()),
            this, SLOT(onOk()));
    connect(pButtonBox, SIGNAL(rejected()),
            this, SLOT(onCancel()));
}


void
ConfigureDialog::onCancel() {
    pTabK236->restoreSettings();
    pTabLS330->restoreSettings();
    pTabCS130->restoreSettings();
    pTabFile->restoreSettings();
    reject();
}



void
ConfigureDialog::onOk() {
    if(pTabFile->checkFileName()) {
        pTabK236->saveSettings();
        pTabLS330->saveSettings();
        pTabCS130->saveSettings();
        pTabFile->saveSettings();
        accept();
    }
    pTabWidget->setCurrentIndex(iFileIndex);
}



