#ifndef LAKESHORE330_H
#define LAKESHORE330_H

#include <QObject>

class LakeShore330 : public QObject
{
  Q_OBJECT
public:
  explicit LakeShore330(int gpio, int address, QObject *parent = 0);
  virtual ~LakeShore330();
  int      Init();
  void     onGpibCallback(int ud, unsigned long ibsta, unsigned long iberr, long ibcntl);
  double   GetTemperature();
  bool     SetTemperature(double Temperature);

signals:

public slots:

private:
  int GPIBNumber;
  int LS330Address;
  int LS330;
  char SpollByte;
  bool bStop;
  int iMask, iComplianceEvents;
  QString sCommand, sResponse;

signals:

public slots:
};

#endif // LAKESHORE330_H
