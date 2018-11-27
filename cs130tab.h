#pragma once

#include <QObject>
#include <QWidget>
#include <QLineEdit>
#include <QRadioButton>
#include <QCheckBox>


class CS130Tab : public QWidget
{
    Q_OBJECT
public:
    explicit CS130Tab(int iConfiguration, QWidget *parent = nullptr);
    void restoreSettings();
    void saveSettings();

signals:

public slots:
    void on_WavelengthEdit_textChanged(const QString &arg1);
    void on_darkPhotoCheck_Clicked(int newState);
    void on_grating1_Selected();
    void on_grating2_Selected();

public:

    double dWavelength;
    int    iGratingNumber;
    bool   bPhoto;
    bool   bDummy[3];

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
    QLineEdit WavelengthEdit;
    QRadioButton radioButtonGrating1;
    QRadioButton radioButtonGrating2;
    QCheckBox darkPhotoCheck;

    const double wavelengthMin;
    const double wavelengthMax;

    int          myConfiguration;
};
