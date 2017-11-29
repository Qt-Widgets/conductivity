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
#ifdef Q_OS_LINUX
#include <gpib/ib.h>
#else
#include <ni4882.h>
#endif


namespace
lakeshore330 {
int rearmMask;
#ifndef Q_OS_LINUX
int __stdcall
myCallback(int LocalUd, unsigned long LocalIbsta, unsigned long LocalIberr, long LocalIbcntl, void* callbackData) {
    reinterpret_cast<LakeShore330*>(callbackData)->onGpibCallback(LocalUd, LocalIbsta, LocalIberr, LocalIbcntl);
    return rearmMask;
  }
#endif
}


LakeShore330::LakeShore330(int gpio, int address, QObject *parent)
  : QObject(parent)
  , gpibNumber(gpio)
  , ls330Address(address)
  , ls330(-1)
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
//  qDebug() << "LakeShore330::~LakeShore330()";
  if(ls330 != -1) {
    switchPowerOff();
#ifdef Q_OS_LINUX
#else
    ibnotify (ls330, 0, NULL, NULL);// disable notification
#endif
    ibonl(ls330, 0);// Disable hardware and software.
  }
}

int
LakeShore330::init() {
//  qDebug() << "LakeShore330::init()";
  ls330 = ibdev(gpibNumber, ls330Address, 0, T3s, 1, 0);
  if(ls330 < 0) {
    qCritical() << "LakeShore330::init() ibdev() Failed";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qCritical() << sError;
    return GPIB_DEVICE_NOT_PRESENT;
  }
  short listen;
  ibln(gpibNumber, ls330Address, NO_SAD, &listen);
  if(isGpibError("LakeShore330::init() LakeShore 330 Not Respondig"))
    return GPIB_DEVICE_NOT_PRESENT;
  if(listen == 0) {
    ibonl(ls330, 0);
    qCritical() << "LakeShore330::init() Nolistener at Addr";
    return GPIB_DEVICE_NOT_PRESENT;
  }
#ifdef Q_OS_LINUX
  connect(&pollTimer, SIGNAL(timeout()),
          this, SLOT(checkNotify()));
  pollTimer.start(10);
#else
  // set up the asynchronous event notification routine on RQS
  ibnotify(ls330,
           RQS,
           (GpibNotifyCallback_t) lakeshore330::myCallback,
           this);
  if(isGpibError("LakeShore330::init() ibnotify call failed."))
    return -1;
#endif
  ibclr(ls330);
  QThread::sleep(1);
  gpibWrite(ls330, "*sre 0\r\n");// Set Service Request Enable
    if(isGpibError("LakeShore330::init(): *sre Failed"))
      return -1;
  gpibWrite(ls330, "RANG 0\r\n");// Sets heater status: 0 = off
  if(isGpibError("LakeShore330::init(): RANG 0 Failed"))
    return -1;
  gpibWrite(ls330, "CUNI K\r\n");// Set Units (Kelvin) for the Control Channel
  if(isGpibError("LakeShore330::init(): CUNI Failed"))
      return -1;
  gpibWrite(ls330, "SUNI K\r\n");// Set Units (Kelvin) for the Sample Channel.
  if(isGpibError("LakeShore330::init(): SUNI Failed"))
    return -1;
  gpibWrite(ls330, "RAMP 0\r\n");// Disables the ramping function
  if(isGpibError("LakeShore330::init(): RAMP 0 Failed"))
    return -1;
  gpibWrite(ls330, "TUNE 4\r\n");// Sets Autotuning Status to "Zone"
  if(isGpibError("LakeShore330::init(): TUNE 4 Failed"))
    return -1;
  return 0;
}


void
LakeShore330::onGpibCallback(int LocalUd, unsigned long LocalIbsta, unsigned long LocalIberr, long LocalIbcntl) {
  Q_UNUSED(LocalUd)
  Q_UNUSED(LocalIbsta)
  Q_UNUSED(LocalIberr)
  Q_UNUSED(LocalIbcntl)
//  qDebug() << "LakeShore330::onGpibCallback()";

  quint8 spollByte;
  ibrsp(ls330, (char *)&spollByte);
  if(isGpibError("LakeShore330::onGpibCallback(): ibrsp() Failed"))
    return;
//  qDebug() << "spollByte=" << spollByte;
  if(spollByte & ESB) {
//    qDebug() << "LakeShore330::onGpibCallback(): Standard Event Status";
    gpibWrite(ls330, "*ESR?\r\n");// Query Std. Event Status Register
    if(isGpibError("LakeShore330::onGpibCallback(): *ESR? Failed"))
      return;
    quint8 esr = quint8(gpibRead(ls330).toInt());
    if(isGpibError("LakeShore330::onGpibCallback(): Read of Std. Event Status Register Failed"))
      return;
    if(esr & PON) {
//      qDebug() << "LakeShore330::onGpibCallback(): Event Status Register PON";
    }
    if(esr & CME) {
//      qDebug() << "LakeShore330::onGpibCallback(): Event Status Register CME";
    }
    if(esr & EXE) {
//      qDebug() << "LakeShore330::onGpibCallback(): Event Status Register EXE";
    }
    if(esr & DDE) {
//      qDebug() << "LakeShore330::onGpibCallback(): Event Status Register DDE";
    }
    if(esr & QYE) {
//      qDebug() << "LakeShore330::onGpibCallback(): Event Status Register QYE";
    }
    if(esr & OPC) {
//      qDebug() << "LakeShore330::onGpibCallback(): Event Status Register OPC";
    }
  }
  if(spollByte & OVI) {
    qCritical() << "LakeShore330::onGpibCallback(): Overload Indicator";
  }
  if(spollByte & CLE) {
    qCritical() << "LakeShore330::onGpibCallback(): Control Limit Error";
  }
  if(spollByte & CDR) {
    qCritical() << "LakeShore330::onGpibCallback(): Control Data Ready";
  }
  if(spollByte & SDR) {
    qCritical() << "LakeShore330::onGpibCallback(): Sample Data Ready";
  }
}


double
LakeShore330::getTemperature() {
//  qDebug() << "LakeShore330::getTemperature()";
  gpibWrite(ls330, "SDAT?\r\n");// Query the Sample Sensor Data.
  if(isGpibError("LakeShore330::getTemperature(): SDAT? Failed"))
    return 0.0;
  sResponse = gpibRead(ls330);
  if(isGpibError("LakeShore330::getTemperature(): ibrd() Failed"))
    return 0.0;
  return sResponse.toDouble();
}


bool
LakeShore330::setTemperature(double Temperature) {
//  qDebug() << QString("LakeShore330::setTemperature(%1)").arg(Temperature);
  if(Temperature < 0.0 || Temperature > 900.0) return false;
  sCommand = QString("SETP %1\r\n").arg(Temperature, 0, 'f', 2);
  gpibWrite(ls330, sCommand);// Sets the Setpoint
  if(isGpibError("LakeShore330::onGpibCallback(): setTemperature(LakeShore): SETP Failed"))
     return false;
  return true;
}


bool
LakeShore330::switchPowerOn(int iRange) {
//  qDebug() << QString("LakeShore330::switchPowerOn(%1)").arg(iRange);
//  sCommand = QString("*SRE %1\r\n").arg(SRQ | ESB | OVI | CLE | CDR | SDR);
//  gpibWrite(ls330, sCommand);
  // Sets heater status: 1 = low, 2 = medium, 3 = high.
  gpibWrite(ls330, QString("RANG %1\r\n").arg(iRange));
  if(isGpibError(QString("LakeShore330::switchPowerOn(%1): Failed").arg(iRange)))
     return false;
  gpibWrite(ls330, "RANG?\r\n");
  if(iRange != gpibRead(ls330).toInt()) {
    qCritical() << "Error setting Heater Range";
  }
  return true;
}


bool
LakeShore330::switchPowerOff() {
//  qDebug() << "LakeShore330::switchPowerOff()";
  gpibWrite(ls330, "*SRE 0\r\n");// Set Service Request Enable to No SRQ
  // Sets heater status: 0 = off.
  gpibWrite(ls330, "RANG 0\r\n");
  if(isGpibError(QString("LakeShore330::switchPowerOff(): Failed")) )
     return false;
  return true;
}


bool
LakeShore330::startRamp(double targetT, double rate) {
  sCommand = QString("RAMPR %1\r\n").arg(rate);
  gpibWrite(ls330, sCommand);
  if(isGpibError("Unable to set Ramp Rate"))
    return false;
  if(!setTemperature(targetT))
    return false;
  sCommand = QString("RAMP 1\r\n");
  gpibWrite(ls330, sCommand);
  QThread::sleep(1);
  if(isGpibError("Unable to Start Ramp"))
    return false;
//  qDebug() << QString("LakeShore330::startRamp(%1)").arg(rate);
  return true;
}


bool
LakeShore330::isRamping() {
  sCommand = QString("RAMPS?\r\n");
  gpibWrite(ls330, sCommand);
  if(isGpibError("Unable to query Ramp Status"))
    return false;
  int iRamping = gpibRead(ls330).toInt();
  if(isGpibError("Unable to get Ramp Status"))
    return false;
  return (iRamping == 1);
}


#ifdef Q_OS_LINUX
void
LakeShore330::checkNotify() {
  ibrsp(ls330, &spollByte);
  if(!(spollByte & 64))
    return; // SRQ not enabled
  onGpibCallback(ls330, ThreadIbsta(), ThreadIberr(), ThreadIbcnt());
}
#endif
