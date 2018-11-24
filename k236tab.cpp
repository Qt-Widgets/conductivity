#include "k236tab.h"

#include <QLineEdit>
#include <QLabel>
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
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);

    pIStartEdit      = new QLineEdit(this);
    pIStopEdit       = new QLineEdit(this);
    pVStartEdit      = new QLineEdit(this);
    pVStopEdit       = new QLineEdit(this);
    pWaitTimeEdit    = new QLineEdit(this);
    pSweepPointsEdit = new QLineEdit(this);

    QGridLayout* pLayout = new QGridLayout();
    // Labels
    pLayout->addWidget(new QLabel("I Start [A]"),     0, 0, 1, 1);
    pLayout->addWidget(new QLabel("I Stop [A]"),      1, 0, 1, 1);
    pLayout->addWidget(new QLabel("V Start [V]"),     2, 0, 1, 1);
    pLayout->addWidget(new QLabel("V Stop [V]"),      3, 0, 1, 1);
    pLayout->addWidget(new QLabel("Rdgs Intv [ms]"),  4, 0, 1, 1);
    pLayout->addWidget(new QLabel("N°of Points"),     5, 0, 1, 1);
    //Line Edits
    pLayout->addWidget(pIStartEdit,      0, 1, 1, 1);
    pLayout->addWidget(pIStopEdit,       1, 1, 1, 1);
    pLayout->addWidget(pVStartEdit,      2, 1, 1, 1);
    pLayout->addWidget(pVStopEdit,       3, 1, 1, 1);
    pLayout->addWidget(pWaitTimeEdit,    4, 1, 1, 1);
    pLayout->addWidget(pSweepPointsEdit, 5, 1, 1, 1);

    sNormalStyle = pIStartEdit->styleSheet();
    sErrorStyle  = "QLineEdit { color: rgb(255, 255, 255); background: rgb(255, 0, 0); selection-background-color: rgb(128, 128, 255); }";

    restoreSettings();
    setToolTips();
    setLayout(pLayout);
}


void
K236Tab::closeEvent(QCloseEvent *event) {
    Q_UNUSED(event)
    saveSettings();
}


void
K236Tab::restoreSettings() {
    QSettings settings;
    restoreGeometry(settings.value("K236TabgGeometry").toByteArray());
    dIStart        = settings.value("K236TabIStart", 0.0).toDouble();
    dIStop         = settings.value("K236TabIStop", 0.0).toDouble();
    dVStart        = settings.value("K236TabVStart", 0.0).toDouble();
    dVStop         = settings.value("K236TabVStop", 0.0).toDouble();
    iWaitTime      = settings.value("K236TabWaitTime", 100).toInt();
    iNSweepPoints  = settings.value("K236TabSweepPoints", 100).toInt();
}


void
K236Tab::saveSettings() {
    QSettings settings;
    settings.setValue("K236TabIStart", dIStart);
    settings.setValue("K236TabIStop", dIStop);
    settings.setValue("K236TabVStart", dVStart);
    settings.setValue("K236TabVStop", dVStop);
    settings.setValue("K236TabWaitTime", iWaitTime);
    settings.setValue("K236TabSweepPoints", iNSweepPoints);
}


void
K236Tab::setToolTips() {
    QString sHeader = QString("Enter values in range [%1 : %2]");
    pIStartEdit->setToolTip(sHeader.arg(currentMin).arg(currentMax));
    pIStopEdit->setToolTip(sHeader.arg(currentMin).arg(currentMax));
    pVStartEdit->setToolTip(sHeader.arg(voltageMin).arg(voltageMax));
    pVStopEdit->setToolTip(sHeader.arg(voltageMin).arg(voltageMax));
    pWaitTimeEdit->setToolTip(sHeader.arg(waitTimeMin).arg(waitTimeMax));
    pSweepPointsEdit->setToolTip((sHeader.arg(nSweepPointsMin).arg(nSweepPointsMax)));
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
K236Tab::onIStartEdit_textChanged(const QString &arg1) {
    if(isCurrentValid(arg1.toDouble())){
        dIStart = arg1.toDouble();
        pIStartEdit->setStyleSheet(sNormalStyle);
    }
    else {
        pIStartEdit->setStyleSheet(sErrorStyle);
    }
}


void
K236Tab::onIStopEdit_textChanged(const QString &arg1) {
    if(isCurrentValid(arg1.toDouble())){
        dIStop = arg1.toDouble();
        pIStopEdit->setStyleSheet(sNormalStyle);
    }
    else {
        pIStopEdit->setStyleSheet(sErrorStyle);
    }
}


void
K236Tab::onVStartEdit_textChanged(const QString &arg1) {
    if(isVoltageValid(arg1.toDouble())){
        dVStart = arg1.toDouble();
        pVStartEdit->setStyleSheet(sNormalStyle);
    }
    else {
        pVStartEdit->setStyleSheet(sErrorStyle);
    }
}


void
K236Tab::onVStopEdit_textChanged(const QString &arg1) {
    if(isVoltageValid(arg1.toDouble())){
        dVStop = arg1.toDouble();
        pVStopEdit->setStyleSheet(sNormalStyle);
    }
    else {
        pVStopEdit->setStyleSheet(sErrorStyle);
    }
}


void
K236Tab::onWaitTimeEdit_textChanged(const QString &arg1) {
    if(isWaitTimeValid(arg1.toInt())) {
        iWaitTime = arg1.toInt();
        pWaitTimeEdit->setStyleSheet(sNormalStyle);
    }
    else {
        pWaitTimeEdit->setStyleSheet(sErrorStyle);
    }
}


void
K236Tab::onSweepPointsEdit_textChanged(const QString &arg1) {
    if(isSweepPointNumberValid(arg1.toInt())) {
        iNSweepPoints = arg1.toInt();
        pSweepPointsEdit->setStyleSheet(sNormalStyle);
    }
    else {
        pSweepPointsEdit->setStyleSheet(sErrorStyle);
    }
}


