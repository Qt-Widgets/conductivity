#ifndef KEITHLEY236_H
#define KEITHLEY236_H

#include <QObject>

class Keithley236 : public QObject
{
  Q_OBJECT
public:
  explicit Keithley236(int gpio, int address, QObject *parent = 0);
  virtual ~Keithley236();
  int      Init();
  void     onGpibCallback(int ud, unsigned long ibsta, unsigned long iberr, long ibcntl);

signals:

public slots:

private:
  int GPIBNumber;
  int K236Address;
  int K236;
  char SpollByte;
  bool bStop;
  int iMask, iComplianceEvents;
  QString sCommand, sResponse;
};

#endif // KEITHLEY236_H
