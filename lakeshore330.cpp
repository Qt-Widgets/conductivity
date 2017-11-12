/*
 *
Copyright (C) 2016  Gabriele Salvato

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/
#include "lakeshore330.h"

#include "utility.h"
#include <QDebug>
#include <QThread>
#include <ni4882.h>

namespace
lakeshore330 {
  int RearmMask;

  int __stdcall
  MyCallback(int LocalUd, unsigned long LocalIbsta, unsigned long LocalIberr, long LocalIbcntl, void* callbackData) {
    reinterpret_cast<LakeShore330*>(callbackData)->onGpibCallback(LocalUd, LocalIbsta, LocalIberr, LocalIbcntl);
    return RearmMask;
  }
}


LakeShore330::LakeShore330(int gpio, int address, QObject *parent)
  : QObject(parent)
  , GPIBNumber(gpio)
  , LS330Address(address)
  , LS330(-1)
{
}


LakeShore330::~LakeShore330() {
  if(LS330 != -1) {
    ibnotify (LS330, 0, NULL, NULL);// disable notification
    ibonl(LS330, 0);// Disable hardware and software.
  }
}

int
LakeShore330::Init() {
  LS330 = ibdev(GPIBNumber, LS330Address, 0, T100ms, 1, 0x1c0A);
  if(LS330 < 0) {
    qDebug() << "ibdev() Failed";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return GPIB_DEVICE_NOT_PRESENT;
  }
  short listen;
  ibln(LS330, LS330Address, NO_SAD, &listen);
  if(ThreadIbsta() & ERR) {
    qDebug() << "LakeShore 330 Not Respondig";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return GPIB_DEVICE_NOT_PRESENT;
  }
  if(listen == 0) {
    ibonl(LS330, 0);
    qDebug() << "Nolistener at Addr";
    return GPIB_DEVICE_NOT_PRESENT;
  }
  // set up the asynchronous event notification routine on RQS
  ibnotify(LS330,
           RQS,
           (GpibNotifyCallback_t) lakeshore330::MyCallback,
           this);
  if(ThreadIbsta() & ERR) {
    qDebug() << "ibnotify call failed.";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
  }
  ibclr(LS330);
  QThread::sleep(1);
  ibrsp(LS330, &SpollByte);
  if(ThreadIbsta() & ERR)  {
    qDebug() << "ibrsp failed";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return -1;
  }
  gpibWrite(LS330, "*sre 0\n");
  if(ThreadIbsta() & ERR) {
    qDebug() << "Init(LakeShore): Failed";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return -1;
  }

  gpibWrite(LS330, "RANG 0\r\n");
  if (ThreadIbsta() & ERR) {
    qDebug() << "Init(LakeShore): Failed";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return -1;
  }
  gpibWrite(LS330, "CUNI K\r\n");
  if (ThreadIbsta() & ERR) {
    qDebug() << "Init(LakeShore): Failed";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return -1;
  }
  gpibWrite(LS330, "SUNI K\r\n");
  if (ThreadIbsta() & ERR) {
    qDebug() << "Init(LakeShore): Failed";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return -1;
  }
  gpibWrite(LS330, "RAMP 0\r\n");
  if (ThreadIbsta() & ERR) {
    qDebug() << "Init(LakeShore): Failed";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return -1;
  }
  gpibWrite(LS330, "TUNE 4\r\n");
  if (ThreadIbsta() & ERR) {
    qDebug() << "Init(LakeShore): Failed";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return -1;
  }
  return 0;
}


void
LakeShore330::onGpibCallback(int LocalUd, unsigned long LocalIbsta, unsigned long LocalIberr, long LocalIbcntl) {

}


double
LakeShore330::GetTemperature() {
  gpibWrite(LS330, "SDAT?\r\n");
  if(ThreadIbsta() & ERR) {
    qDebug() << "SDAT?\r\nInit(LakeShore): Failed";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return 0.0;
  }
  sResponse = gpibRead(LS330);
  if (ThreadIbsta() & ERR) {
    qDebug() << "GetTemp(LakeShore): ibrd() Failed";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return 0.0;
  } else {
    return sResponse.toDouble();
  }
}


bool
LakeShore330::SetTemperature(double Temperature) {
  if(Temperature < 0.0 || Temperature > 900.0) return false;
  gpibWrite(LS330, "TUNE 3\r\n");
  sCommand = QString("SETP %1\r\n").arg(Temperature, 0, 'f', 2);
  gpibWrite(LS330, sCommand);
  if (ThreadIbsta() & ERR) {
    qDebug() << "SETP\r\nSetTemp(LakeShore): Failed";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return false;
  }
  return true;
}


bool
LakeShore330::switchPowerOn() {
  qCritical() << "Fake switchPowerOn()";
  return true;
}


bool
LakeShore330::switchPowerOff() {
  qCritical() << "Fake switchPowerOff()";
  return true;
}
