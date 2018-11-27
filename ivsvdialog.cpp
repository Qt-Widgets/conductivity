#include "ivsvdialog.h"

#include <QTabWidget>
#include <QVBoxLayout>
#include <QtDebug>


IvsVDialog::IvsVDialog(QWidget *parent)
    : QDialog(parent)
{
    setAttribute(Qt::WA_AlwaysShowToolTips);
    pTabK236  = new K236Tab(1, this);//1 => IvsV configuration
    pTabLS330 = new LS330Tab(1, this);
    pTabCS130 = new CS130Tab(1, this);
    pTabFile  = new FileTab(1, this);

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
IvsVDialog::setToolTips() {
    pTabWidget->setTabToolTip(iSourceIndex, QString("Source-Measure Unit configuration"));
    pTabWidget->setTabToolTip(iThermIndex, QString("Thermostat configuration"));
    pTabWidget->setTabToolTip(iMonoIndex, QString("Monochromator configuration"));
    pTabWidget->setTabToolTip(iFileIndex, QString("Output File configuration"));
}


void
IvsVDialog::connectSignals() {
    connect(pButtonBox, SIGNAL(accepted()),
            this, SLOT(onOk()));
    connect(pButtonBox, SIGNAL(rejected()),
            this, SLOT(onCancel()));
}


void
IvsVDialog::onCancel() {
    pTabK236->restoreSettings();
    pTabLS330->restoreSettings();
    pTabCS130->restoreSettings();
    pTabFile->restoreSettings();
    reject();
}



void
IvsVDialog::onOk() {
    if(pTabFile->checkFileName()) {
        pTabK236->saveSettings();
        pTabLS330->saveSettings();
        pTabCS130->saveSettings();
        pTabFile->saveSettings();
        accept();
    }
    pTabWidget->setCurrentIndex(iFileIndex);
}



