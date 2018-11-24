#pragma once

#include <QObject>
#include <QWidget>


QT_FORWARD_DECLARE_CLASS(QLineEdit)


class K236Tab : public QWidget
{
    Q_OBJECT
public:
    explicit K236Tab(QWidget *parent = nullptr);

signals:

public slots:
    void onIStartEdit_textChanged(const QString &arg1);
    void onIStopEdit_textChanged(const QString &arg1);
    void onVStartEdit_textChanged(const QString &arg1);
    void onVStopEdit_textChanged(const QString &arg1);
    void onWaitTimeEdit_textChanged(const QString &arg1);
    void onSweepPointsEdit_textChanged(const QString &arg1);

protected:
    void restoreSettings();
    void saveSettings();
    void setToolTips();
    void closeEvent(QCloseEvent *event);
    void setCaptions();
    bool isCurrentValid(double dCurrent);
    bool isVoltageValid(double dVoltage);
    bool isWaitTimeValid(int iWaitTime);
    bool isSweepPointNumberValid(int nSweepPoints);

public:
    double dIStart;
    double dIStop;
    double dVStart;
    double dVStop;
    int    iWaitTime;
    int    iNSweepPoints;

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
    QString     sNormalStyle;
    QString     sErrorStyle;

    QLineEdit   *pIStartEdit;
    QLineEdit   *pIStopEdit;
    QLineEdit   *pVStartEdit;
    QLineEdit   *pVStopEdit;
    QLineEdit   *pWaitTimeEdit;
    QLineEdit   *pSweepPointsEdit;
};

