#include "keithley236.h"
#include "utility.h"
#include <QDebug>
#include <QThread>
#include <ni4882.h>

#define MAX_COMPLIANCE_EVENTS 5

namespace keithley236 {
  int RearmMask;
  int __stdcall
  MyCallback(int LocalUd, unsigned long LocalIbsta, unsigned long LocalIberr, long LocalIbcntl, void* callbackData) {
    reinterpret_cast<Keithley236*>(callbackData)->onGpibCallback(LocalUd, LocalIbsta, LocalIberr, LocalIbcntl);
    return RearmMask;
  }
}


Keithley236::Keithley236(int gpio, int address, QObject *parent)
  : QObject(parent)
  , GPIBNumber(gpio)
  , K236Address(address)
  , K236(-1)
{
}


Keithley236::~Keithley236() {
  if(K236 != -1) {
    ibnotify (K236, 0, NULL, NULL);// disable notification
    ibonl(K236, 0);// Disable hardware and software.
  }
}

int
Keithley236::Init() {
  K236 = ibdev(GPIBNumber, K236Address, 0, T100ms, 1, 0);
  if(K236 < 0) {
    qDebug() << "ibdev() Failed";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return GPIB_DEVICE_NOT_PRESENT;
  }
  short listen;
  ibln(K236, K236Address, NO_SAD, &listen);
  if(ibsta & ERR) {
    qDebug() << "Keithley 236 Not Respondig";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return GPIB_DEVICE_NOT_PRESENT;
  }
  if(listen == 0) {
    ibonl(K236, 0);
    qDebug() << "Nolistener at Addr";
    return GPIB_DEVICE_NOT_PRESENT;
  }
  // set up the asynchronous event notification routine on RQS
  ibnotify(K236,
           RQS,
           (GpibNotifyCallback_t) keithley236::MyCallback,
           this);
  if(ibsta & ERR) {
    qDebug() << "ibnotify call failed.";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
  }
  ibclr(K236);
  QThread::sleep(1);
  ibrsp(K236, &SpollByte);
  if(ibsta & ERR)  {
    sCommand = QString("ibrsp failed");
    qDebug() << sCommand;
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return -1;
  }
  return 0;
}


void
Keithley236::onGpibCallback(int LocalUd, unsigned long LocalIbsta, unsigned long LocalIberr, long LocalIbcntl) {

}
