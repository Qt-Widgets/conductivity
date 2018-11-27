#pragma once

#include <QObject>
#include <QWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>


class LS330Tab : public QWidget
{
    Q_OBJECT
public:
    explicit LS330Tab(QWidget *parent = nullptr);
    ~LS330Tab() Q_DECL_OVERRIDE;
    void restoreSettings();
    void saveSettings();
    void connectSignals();

public:
    double dTStart;
    double dTStop;
    double dTStep;
    int    iReachingTStart;
    int    iTimeToSteadyT;
    bool   bUseThermostat;

signals:

public slots:
    void on_ThermostatCheckBox_stateChanged(int arg1);
    void on_TStartEdit_textChanged(const QString &arg1);
    void on_TStopEdit_textChanged(const QString &arg1);
    void on_TStepEdit_textChanged(const QString &arg1);
    void on_MaxTimeToTStartEdit_textChanged(const QString &arg1);
    void on_TimeToSteadyTEdit_textChanged(const QString &arg1);

protected:
    void initUI();
    void setToolTips();
    bool isTemperatureValid(double dTemperature);
    bool isTStepValid(double dTStep);
    bool isReachingTimeValid(int iReachingTime);
    bool isTimeToSteadyTValid(int iTime);

private:
    // QLineEdit styles
    QString sNormalStyle;
    QString sErrorStyle;

    QLineEdit TStartEdit;
    QLineEdit TStopEdit;
    QLineEdit TStepEdit;
    QLineEdit MaxTimeToTStartEdit;
    QLineEdit TimeToSteadyTEdit;

    QCheckBox ThermostatCheckBox;


    const double temperatureMin;
    const double temperatureMax;
    const int    waitTimeMin;
    const int    waitTimeMax;
    const int    reachingTMin;
    const int    reachingTMax;
    const int    timeToSteadyTMin;
    const int    timeToSteadyTMax;
};
