#include "cs130tab.h"

#include <QGridLayout>
#include <QSettings>

CS130Tab::CS130Tab(QWidget *parent)
    : QWidget(parent)
    , wavelengthMin(200.0)
    , wavelengthMax(1000.0)
{
    // Build the Tab layout
    QGridLayout* pLayout = new QGridLayout();

    setLayout(pLayout);

//    sNormalStyle = pStartEdit->styleSheet();

    sErrorStyle  = "QLineEdit { ";
    sErrorStyle += "color: rgb(255, 255, 255);";
    sErrorStyle += "background: rgb(255, 0, 0);";
    sErrorStyle += "selection-background-color: rgb(128, 128, 255);";
    sErrorStyle += "}";

    connectSignals();
    restoreSettings();
    initUI();
}

void
CS130Tab::restoreSettings() {
    QSettings settings;
}


void
CS130Tab::saveSettings() {
    QSettings settings;
}


void
CS130Tab::setToolTips() {
    QString sHeader = QString("Enter values in range [%1 : %2]");
}


void
CS130Tab::initUI() {
}


void
CS130Tab::connectSignals() {
}

