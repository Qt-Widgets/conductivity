#pragma once

#include <QObject>
#include <QWidget>

class CS130Tab : public QWidget
{
    Q_OBJECT
public:
    explicit CS130Tab(QWidget *parent = nullptr);
    void restoreSettings();
    void saveSettings();

signals:

public slots:
    void on_WavelengthEdit_textChanged(const QString &arg1);
    void on_darkPhotoCheck_clicked();

public:

    double dWavelength;
    int    iGratingNumber;
    bool   bPhoto;

protected:
    void initUI();
    void setToolTips();
    void connectSignals();
    bool isWavelengthValid(double wavelength);
    void enableMonochromator(bool bEnable);

private:
    // QLineEdit styles
    QString sNormalStyle;
    QString sErrorStyle;
    const double wavelengthMin;
    const double wavelengthMax;
};
