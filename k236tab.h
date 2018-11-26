#pragma once

#include <QObject>
#include <QWidget>


QT_FORWARD_DECLARE_CLASS(QLineEdit)
QT_FORWARD_DECLARE_CLASS(QRadioButton)
QT_FORWARD_DECLARE_CLASS(QLabel)


class K236Tab : public QWidget
{
    Q_OBJECT
public:
    explicit K236Tab(QWidget *parent = nullptr);
    void restoreSettings();
    void saveSettings();

signals:

public slots:
    void onSourceIChecked();
    void onSourceVChecked();
    void onStartEdit_textChanged(const QString &arg1);
    void onStopEdit_textChanged(const QString &arg1);
    void onComplianceEdit_textChanged(const QString &arg1);
    void onWaitTimeEdit_textChanged(const QString &arg1);
    void onSweepPointsEdit_textChanged(const QString &arg1);

protected:
    void setToolTips();
    void setCaptions();
    void initUI();
    void connectSignals();
    bool isCurrentValid(double dCurrent);
    bool isVoltageValid(double dVoltage);
    bool isComplianceValid(double dCompliance);
    bool isWaitTimeValid(int iWaitTime);
    bool isSweepPointNumberValid(int nSweepPoints);

public:
    double dStart;
    double dStop;
    double dCompliance;
    int    iWaitTime;
    int    iNSweepPoints;
    bool   bSourceI;

private:
    // Limit Values
    const double currentMin;
    const double currentMax;
    const double voltageMin;
    const double voltageMax;
    const int    waitTimeMin;
    const int    waitTimeMax;
    const int    nSweepPointsMin;
    const int    nSweepPointsMax;

    // QLineEdit styles
    QString sNormalStyle;
    QString sErrorStyle;

    // UI Elements
    QRadioButton *pSourceIButton;
    QRadioButton *pSourceVButton;
    QLabel       *pStartLabel;
    QLabel       *pStopLabel;
    QLabel       *pComplianceLabel;
    QLineEdit    *pStartEdit;
    QLineEdit    *pStopEdit;
    QLineEdit    *pComplianceEdit;
    QLineEdit    *pWaitTimeEdit;
    QLineEdit    *pSweepPointsEdit;
};

