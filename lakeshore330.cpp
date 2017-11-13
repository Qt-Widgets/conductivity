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
  // Status Byte bits
  , SRQ(64)// Service Request
  , ESB(32)// Standard Event Status
  , OVI(16)// Overload Indicator
  , CLE(4) // Control Limit Ready
  , CDR(2) // Control Data Ready
  , SDR(1) // Sample Data Ready
  // Standard Event Status Register bits
  , PON(128)// Power On
  , CME(32) // Command Error
  , EXE(16) // Execution Error
  , DDE(8)  // Device Dependent Error
  , QYE(4)  // Query Error
  , OPC(1)  // Operation Complete
{

}


LakeShore330::~LakeShore330() {
  qDebug() << "LakeShore330::~LakeShore330()";
  if(LS330 != -1) {
    ibnotify (LS330, 0, NULL, NULL);// disable notification
    ibonl(LS330, 0);// Disable hardware and software.
  }
}

int
LakeShore330::init() {
  qDebug() << "LakeShore330::init()";
  LS330 = ibdev(GPIBNumber, LS330Address, 0, T100ms, 1, 0x1c0A);
  if(LS330 < 0) {
    qDebug() << "LakeShore330::init() ibdev() Failed";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return GPIB_DEVICE_NOT_PRESENT;
  }
  short listen;
  ibln(LS330, LS330Address, NO_SAD, &listen);
  if(ThreadIbsta() & ERR) {
    qDebug() << "LakeShore330::init() LakeShore 330 Not Respondig";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return GPIB_DEVICE_NOT_PRESENT;
  }
  if(listen == 0) {
    ibonl(LS330, 0);
    qDebug() << "LakeShore330::init() Nolistener at Addr";
    return GPIB_DEVICE_NOT_PRESENT;
  }
  // set up the asynchronous event notification routine on RQS
  ibnotify(LS330,
           RQS,
           (GpibNotifyCallback_t) lakeshore330::MyCallback,
           this);
  if(ThreadIbsta() & ERR) {
    qDebug() << "LakeShore330::init() ibnotify call failed.";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
  }
  ibclr(LS330);
  QThread::sleep(1);
  ibrsp(LS330, &SpollByte);
  if(ThreadIbsta() & ERR)  {
    qDebug() << "LakeShore330::init() ibrsp failed";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return -1;
  }
  gpibWrite(LS330, "*sre 0\n");// Set Service Request Enable
  if(ThreadIbsta() & ERR) {
    qDebug() << "LakeShore330::init(): Failed";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return -1;
  }

  gpibWrite(LS330, "RANG 0\r\n");// Sets heater status: 0 = off
  if(ThreadIbsta() & ERR) {
    qDebug() << "LakeShore330::init(): Failed";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return -1;
  }
  gpibWrite(LS330, "CUNI K\r\n");// Set Units (Kelvin) for the Control Channel
  if(ThreadIbsta() & ERR) {
    qDebug() << "LakeShore330::init(): Failed";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return -1;
  }
  gpibWrite(LS330, "SUNI K\r\n");// Set Units (Kelvin) for the Sample Channel.
  if(ThreadIbsta() & ERR) {
    qDebug() << "LakeShore330::init(): Failed";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return -1;
  }
  gpibWrite(LS330, "RAMP 0\r\n");// Disables the ramping function
  if(ThreadIbsta() & ERR) {
    qDebug() << "LakeShore330::init(): Failed";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return -1;
  }
  gpibWrite(LS330, "TUNE 4\r\n");// Sets Autotuning Status to "Zone"
  if(ThreadIbsta() & ERR) {
    qDebug() << "LakeShore330::init(): Failed";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return -1;
  }
  return 0;
}


void
LakeShore330::onGpibCallback(int LocalUd, unsigned long LocalIbsta, unsigned long LocalIberr, long LocalIbcntl) {
  Q_UNUSED(LocalUd)
  Q_UNUSED(LocalIbsta)
  Q_UNUSED(LocalIberr)
  Q_UNUSED(LocalIbcntl)
  qDebug() << "LakeShore330::onGpibCallback()";

  quint8 spollByte;
  ibrsp(LS330, (char *)&spollByte);
  if(ThreadIbsta() & ERR) {
    qDebug() << "LakeShore330::onGpibCallback(): ibrsp() Failed";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return;
  }
  qDebug() << "spollByte=" << spollByte;
  if(spollByte & ESB) {
    qDebug() << "LakeShore330::onGpibCallback(): Standard Event Status";
    // *ESR? Query Std. Event Status Register
  }
  if(spollByte & OVI) {
    qDebug() << "LakeShore330::onGpibCallback(): Overload Indicator";
  }
  if(spollByte & CLE) {
    qDebug() << "LakeShore330::onGpibCallback(): Control Limit Error";
  }
  if(spollByte & CDR) {
    qDebug() << "LakeShore330::onGpibCallback(): Control Data Ready";
  }
  if(spollByte & SDR) {
    qDebug() << "LakeShore330::onGpibCallback(): Sample Data Ready";
  }
}


double
LakeShore330::getTemperature() {
  qDebug() << "LakeShore330::getTemperature()";
  gpibWrite(LS330, "SDAT?\r\n");// Query the Sample Sensor Data.
  if(ThreadIbsta() & ERR) {
    qDebug() << "LakeShore330::getTemperature(): SDAT? Failed";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return 0.0;
  }
  sResponse = gpibRead(LS330);
  if(ThreadIbsta() & ERR) {
    qDebug() << "LakeShore330::getTemperature(): ibrd() Failed";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return 0.0;
  }
  return sResponse.toDouble();
}


bool
LakeShore330::setTemperature(double Temperature) {
  qDebug() << QString("LakeShore330::setTemperature(%1)").arg(Temperature);
  if(Temperature < 0.0 || Temperature > 900.0) return false;
  sCommand = QString("SETP %1\r\n").arg(Temperature, 0, 'f', 2);
  gpibWrite(LS330, sCommand);// Sets the Setpoint
  if(ThreadIbsta() & ERR) {
    qDebug() << "setTemperature(LakeShore): SETP Failed";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return false;
  }
  return true;
}


bool
LakeShore330::switchPowerOn() {
  qDebug() << "LakeShore330::switchPowerOn()";
  sCommand = QString("*SRE%1\r\n").arg(SRQ | ESB | OVI | CLE | CDR | SDR);
  gpibWrite(LS330, sCommand);
  // Sets heater status: 1 = low, 2 = medium, 3 = high.
  gpibWrite(LS330, "RANG 1\r\n");
  if(ThreadIbsta() & ERR) {
    qDebug() << "LakeShore330::init(): Failed";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return false;
  }
  return true;
}


bool
LakeShore330::switchPowerOff() {
  qDebug() << "Fake LakeShore330::switchPowerOff()";
  gpibWrite(LS330, "*sre 0\n");// Set Service Request Enable to No SRQ
  // Sets heater status: 0 = off.
  gpibWrite(LS330, "RANG 0\r\n");
  if(ThreadIbsta() & ERR) {
    qDebug() << "LakeShore330::switchPowerOff(): Failed";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return false;
  }
  return true;
}


bool
LakeShore330::startRamp(double rate) {
  qDebug() << QString("Fake LakeShore330::switchPowerOff(%1)").arg(rate);
  return true;
}
