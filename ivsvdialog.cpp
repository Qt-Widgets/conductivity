#include "ivsvdialog.h"
#include "k236tab.h"
#include "ls330tab.h"
#include "cs130tab.h"

#include <QTabWidget>
#include <QDialogButtonBox>
#include <QVBoxLayout>


IvsVDialog::IvsVDialog(QWidget *parent)
    : QDialog(parent)
{
    tabWidget = new QTabWidget;

    pK236Tab  = new K236Tab();
    pLS330Tab = new LS330Tab();
    pCS130Tab = new CS130Tab();

    tabWidget->addTab(pK236Tab, tr("K236"));
    tabWidget->addTab(pLS330Tab,tr("LS330"));
    tabWidget->addTab(pCS130Tab,tr("CS130"));

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
    pK236Tab->restoreSettings();
    //pLS330Tab->restoreSettings();
    //pCS130Tab->restoreSettings();
    emit reject();
}



void
IvsVDialog::onOk(){
    pK236Tab->saveSettings();
    //pLS330Tab->saveSettings();
    //pCS130Tab->saveSettings();
    emit accept();
}


