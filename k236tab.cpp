#include "k236tab.h"

#include <QLineEdit>
#include <QLabel>
#include <QRadioButton>
#include <QGridLayout>
#include <QSettings>


K236Tab::K236Tab(QWidget *parent)
    : QWidget(parent)
    , currentMin(-1.0e-2)
    , currentMax(1.0e-2)
    , voltageMin(-110.0)
    , voltageMax(110.0)
    , waitTimeMin(100)
    , waitTimeMax(65000)
    , nSweepPointsMin(3)
    , nSweepPointsMax(500)
{
    setAttribute(Qt::WA_AlwaysShowToolTips);
    // Create UI Elements
    pSourceIButton   = new QRadioButton(QString("Source I"), this);
    pSourceVButton   = new QRadioButton(QString("Source V"), this);
    pStartEdit       = new QLineEdit(this);
    pStopEdit        = new QLineEdit(this);
    pComplianceEdit  = new QLineEdit(this);
    pWaitTimeEdit    = new QLineEdit(this);
    pSweepPointsEdit = new QLineEdit(this);

    // Build the Tab layout
    QGridLayout* pLayout = new QGridLayout();
    // Radio Buttons
    pLayout->addWidget(pSourceIButton, 0, 0, 1, 1);
    pLayout->addWidget(pSourceVButton, 0, 1, 1, 1);
    bSourceI = true;
    // Labels
    pStartLabel      = new QLabel("I Start [A]");
    pStopLabel       = new QLabel("I Stop [A]");
    pComplianceLabel = new QLabel("Compliance [V]");
    pLayout->addWidget(pStartLabel,                  1, 0, 1, 1);
    pLayout->addWidget(pStopLabel,                   2, 0, 1, 1);
    pLayout->addWidget(pComplianceLabel,             3, 0, 1, 1);
    pLayout->addWidget(new QLabel("Rdgs Intv [ms]"), 4, 0, 1, 1);
    pLayout->addWidget(new QLabel("N°of Points"),    5, 0, 1, 1);
    //Line Edits
    pLayout->addWidget(pStartEdit,       1, 1, 1, 1);
    pLayout->addWidget(pStopEdit,        2, 1, 1, 1);
    pLayout->addWidget(pComplianceEdit,  3, 1, 1, 1);
    pLayout->addWidget(pWaitTimeEdit,    4, 1, 1, 1);
    pLayout->addWidget(pSweepPointsEdit, 5, 1, 1, 1);

    setLayout(pLayout);

    sNormalStyle = pStartEdit->styleSheet();

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
K236Tab::restoreSettings() {
    QSettings settings;
    bSourceI       = settings.value("K236TabSourceI", true).toBool();
    dStart        = settings.value("K236TabStart", 0.0).toDouble();
    dStop         = settings.value("K236TabStop", 0.0).toDouble();
    dCompliance   = settings.value("K236TabCompliance", 0.0).toDouble();
    iWaitTime      = settings.value("K236TabWaitTime", 100).toInt();
    iNSweepPoints  = settings.value("K236TabSweepPoints", 100).toInt();
}


void
K236Tab::saveSettings() {
    QSettings settings;
    settings.setValue("K236TabSourceI",     bSourceI);
    settings.setValue("K236TabStart",       dStart);
    settings.setValue("K236TabStop",        dStop);
    settings.setValue("K236TabCompliance",  dCompliance);
    settings.setValue("K236TabWaitTime",    iWaitTime);
    settings.setValue("K236TabSweepPoints", iNSweepPoints);
}


void
K236Tab::setToolTips() {
    QString sHeader = QString("Enter values in range [%1 : %2]");
    if(bSourceI) {
        pStartEdit->setToolTip(sHeader.arg(currentMin).arg(currentMax));
        pStopEdit->setToolTip(sHeader.arg(currentMin).arg(currentMax));
        pComplianceEdit->setToolTip(sHeader.arg(voltageMin).arg(voltageMax));
    }
    else {
        pStartEdit->setToolTip(sHeader.arg(voltageMin).arg(voltageMax));
        pStopEdit->setToolTip(sHeader.arg(voltageMin).arg(voltageMax));
        pComplianceEdit->setToolTip(sHeader.arg(currentMin).arg(currentMax));
    }
    pSourceIButton->setToolTip("Source Current - Measure Voltage");
    pSourceVButton->setToolTip("Source Voltage - Measure Current");
    pWaitTimeEdit->setToolTip(sHeader.arg(waitTimeMin).arg(waitTimeMax));
    pSweepPointsEdit->setToolTip((sHeader.arg(nSweepPointsMin).arg(nSweepPointsMax)));
}


void
K236Tab::initUI() {
    // Measurement parameters
    if(bSourceI) {
        pSourceIButton->setChecked(true);
        if(!isCurrentValid(dStart))
            dStart = 0.0;
        if(!isCurrentValid(dStop))
            dStop = 0.0;
        pStartLabel->setText("I Start [A]");
        pStopLabel->setText("I Stop [A]");
        pComplianceLabel->setText("Compliance [V]");
    }
    else {
        pSourceVButton->setChecked(true);
        if(!isVoltageValid(dStart))
            dStart = 0.0;
        if(!isVoltageValid(dStop))
            dStop = 0.0;
        pStartLabel->setText("V Start [V]");
        pStopLabel->setText("V Stop [V]");
        pComplianceLabel->setText("Compliance [A]");
    }
    pStartEdit->setText(QString("%1").arg(dStart, 0, 'g', 2));
    pStopEdit->setText(QString("%1").arg(dStop, 0, 'g', 2));
    if(!isComplianceValid(dCompliance))
        dCompliance = 0.0;
    pComplianceEdit->setText(QString("%1").arg(dCompliance, 0, 'g', 2));
    if(!isWaitTimeValid(iWaitTime))
        iWaitTime = 100;
    pWaitTimeEdit->setText(QString("%1").arg(iWaitTime));
    if(!isSweepPointNumberValid(iNSweepPoints))
        iNSweepPoints = 100;
    pSweepPointsEdit->setText(QString("%1").arg(iNSweepPoints));
    setToolTips();
}


void
K236Tab::connectSignals() {
    connect(pStartEdit, SIGNAL(textChanged(const QString)),
            this, SLOT(onStartEdit_textChanged(const QString)));
    connect(pStopEdit, SIGNAL(textChanged(const QString)),
            this, SLOT(onStopEdit_textChanged(const QString)));
    connect(pComplianceEdit, SIGNAL(textChanged(const QString)),
            this, SLOT(onComplianceEdit_textChanged(const QString)));
    connect(pWaitTimeEdit, SIGNAL(textChanged(const QString)),
            this, SLOT(onWaitTimeEdit_textChanged(const QString)));
    connect(pSweepPointsEdit, SIGNAL(textChanged(const QString)),
            this, SLOT(onSweepPointsEdit_textChanged(const QString)));
    connect(pSourceIButton, SIGNAL(clicked()),
            this, SLOT(onSourceIChecked()));
    connect(pSourceVButton, SIGNAL(clicked()),
            this, SLOT(onSourceVChecked()));
}


bool
K236Tab::isCurrentValid(double dCurrent) {
    return (dCurrent >= currentMin) &&
            (dCurrent <= currentMax);
}


bool
K236Tab::isVoltageValid(double dVoltage) {
    return (dVoltage >= voltageMin) &&
            (dVoltage <= voltageMax);
}


bool
K236Tab::isComplianceValid(double dCompliance){
    if(bSourceI)
        return isCurrentValid(dCompliance);
    else
        return isVoltageValid(dCompliance);
}


bool
K236Tab::isWaitTimeValid(int iWaitTime) {
    return (iWaitTime >= waitTimeMin) &&
            (iWaitTime <= waitTimeMax);
}


bool
K236Tab::isSweepPointNumberValid(int nSweepPoints) {
    return (nSweepPoints >= nSweepPointsMin) &&
            (nSweepPoints <= nSweepPointsMax);
}


void
K236Tab::onStartEdit_textChanged(const QString &arg1) {
    double dTemp = arg1.toDouble();
    bool bValid = bSourceI ? isCurrentValid(dTemp) : isVoltageValid(dTemp);
    if(bValid) {
        dStart = dTemp;
        pStartEdit->setStyleSheet(sNormalStyle);
    }
    else {
        pStartEdit->setStyleSheet(sErrorStyle);
    }
}


void
K236Tab::onStopEdit_textChanged(const QString &arg1) {
    double dTemp = arg1.toDouble();
    bool bValid = bSourceI ? isCurrentValid(dTemp) : isVoltageValid(dTemp);
    if(bValid) {
        dStop = dTemp;
        pStopEdit->setStyleSheet(sNormalStyle);
    }
    else {
        pStopEdit->setStyleSheet(sErrorStyle);
    }
}


void
K236Tab::onComplianceEdit_textChanged(const QString &arg1) {
    double dTemp = arg1.toDouble();
    bool bValid = bSourceI ? isVoltageValid(dTemp) : isCurrentValid(dTemp);
    if(bValid) {
        dCompliance = dTemp;
        pComplianceEdit->setStyleSheet(sNormalStyle);
    }
    else {
        pComplianceEdit->setStyleSheet(sErrorStyle);
    }
}


void
K236Tab::onSourceIChecked() {
    bSourceI = true;
    initUI();
}


void
K236Tab::onSourceVChecked() {
    bSourceI = false;
    initUI();
}


void
K236Tab::onWaitTimeEdit_textChanged(const QString &arg1) {
    int iTemp = arg1.toInt();
    if(isWaitTimeValid(iTemp)) {
        iWaitTime = iTemp;
        pWaitTimeEdit->setStyleSheet(sNormalStyle);
    }
    else {
        pWaitTimeEdit->setStyleSheet(sErrorStyle);
    }
}


void
K236Tab::onSweepPointsEdit_textChanged(const QString &arg1) {
    int iTemp = arg1.toInt();
    if(isSweepPointNumberValid(iTemp)) {
        iNSweepPoints = iTemp;
        pSweepPointsEdit->setStyleSheet(sNormalStyle);
    }
    else {
        pSweepPointsEdit->setStyleSheet(sErrorStyle);
    }
}

