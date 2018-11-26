#include "ivsvdialog.h"

#include <QTabWidget>
#include <QDialogButtonBox>
#include <QVBoxLayout>


IvsVDialog::IvsVDialog(QWidget *parent)
    : QDialog(parent)
{
    tabWidget = new QTabWidget;

    tabWidget->addTab(&TabK236, tr("K236"));
    tabWidget->addTab(&TabLS330,tr("LS330"));
    tabWidget->addTab(&TabCS130,tr("CS130"));
    tabWidget->addTab(&TabFile,tr("Out File"));

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                     QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()),
            this, SLOT(onOk()));
    connect(buttonBox, SIGNAL(rejected()),
            this, SLOT(onCancel()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
    setWindowTitle("I versus V");
}


void
IvsVDialog::onCancel() {
    TabK236.restoreSettings();
    //TabLS330.restoreSettings();
    //TabCS130.restoreSettings();
    TabFile.restoreSettings();
    reject();
}



void
IvsVDialog::onOk() {
    TabK236.saveSettings();
    //LS330Tab.saveSettings();
    //CS130Tab.saveSettings();
    TabFile.saveSettings();
    accept();
}


