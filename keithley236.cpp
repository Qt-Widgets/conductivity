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
  //
  , NO_JUNCTION(0)
  , FORWARD_JUNCTION(1)
  , REVERSE_JUNCTION(-1)
  //
  , SRQ_DISABLED(0)
  , WARNING(1)
  , SWEEP_DONE(2)
  , TRIGGER_OUT(4)
  , READING_DONE(8)
  , READY_FOR_TRIGGER(16)
  , K236_ERROR(32)
  , COMPLIANCE(128)
  //
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


int
Keithley236::InitVvsT(double dAppliedCurrent, double dVoltageCompliance) {
  gpibWrite(K236, "O1");       // Remote Sense
  gpibWrite(K236, "Z0");       // Disable Zero suppression
  gpibWrite(K236, "F1,0");     // Source I Measure V dc
  sCommand = QString("B%1,0,0").arg(dAppliedCurrent);
  gpibWrite(K236, sCommand);
  sCommand = QString("L%g,0").arg(dVoltageCompliance);
  gpibWrite(K236, sCommand);   // Set Compliance, Autorange Measure
  gpibWrite(K236, "H0X");      // Enable Comliance
  gpibWrite(K236, "T1,0,0,0"); // Trigger on GET
  gpibWrite(K236, "G5,2,0");   // Output Source, Measure, No Prefix, DC
  gpibWrite(K236, "S3");       // 20ms integration time
  gpibWrite(K236, "P5X");      // 32 Reading Filter
  keithley236::RearmMask = 0;
  sCommand = QString("M%d,0").arg(keithley236::RearmMask);
  gpibWrite(K236, sCommand);   // SRQ Mask, Interrupt on Compliance
  gpibWrite(K236, "R1");       // Arm Trigger
  gpibWrite(K236, "N1X");      // Operate !
  ibrsp(K236, &SpollByte);     // To Clear SRQ
  keithley236::RearmMask =
      COMPLIANCE +
      K236_ERROR +
      READY_FOR_TRIGGER +
      READING_DONE +
      WARNING;
  sCommand = QString("M%d,0X").arg(keithley236::RearmMask);
  gpibWrite(K236, sCommand);   // SRQ Mask, Interrupt on Compliance
  return 0;
}


int
Keithley236::JunctionCheck() {// Per sapere se abbiamo una giunzione !
  gpibWrite(K236, "F0,0");     // Source V Measure I dc
  gpibWrite(K236, "O0");       // Local Sense
  gpibWrite(K236, "T1,0,0,0"); // Trigger on GET
  gpibWrite(K236, "L1.0e-4,0");// Set Compliance, Autorange Measure
  gpibWrite(K236, "G5,2,0");   // Output Source, Measure, No Prefix, DC
  gpibWrite(K236, "S3");       // 20ms integration time
  gpibWrite(K236, "P5");       // 32 Reading Filter
  keithley236::RearmMask = SRQ_DISABLED;
  sCommand = QString("M%d,0").arg(iMask);
  gpibWrite(K236, sCommand);   // SRQ Disabled
  gpibWrite(K236, "B1.0,0,0"); // Source 1.0V Measure I Autorange
  gpibWrite(K236, "R1");       // Arm Trigger
  gpibWrite(K236, "Z0");       // Disable suppression
  gpibWrite(K236, "N1X");      // Operate !
  ibrsp(K236, &SpollByte);
  while(!(SpollByte & 16)) {// Ready for trigger
    ibrsp(K236, &SpollByte);
    QThread::msleep(100);
  }
  ibtrg(K236);
  ibrsp(K236, &SpollByte);
  while(!(SpollByte & 8)) {// Reading Done
    ibrsp(K236, &SpollByte);
    QThread::msleep(100);
  }
  sCommand = gpibRead(K236);
  double I_Forward = sCommand.toDouble();
  qDebug() << QString("Forward Current [A]= ") + sCommand;
  gpibWrite(K236, "B-1.0,0,0X"); // Source -1.0V Measure I Autorange
  QThread::msleep(1000);
  sCommand = gpibRead(K236);
  double I_Reverse = sCommand.toDouble();
  qDebug() << QString("Reverse Current [A]= ") + sCommand;
  gpibWrite(K236, "B0.0,0,0"); // Source 0.0V Measure I Autorange
  gpibWrite(K236, "N0X");      // Stand By !
  int deltaI = abs(qRound(log10(abs(I_Forward))) - qRound(log10(abs(I_Reverse))));
  if(deltaI < 2) return NO_JUNCTION;// No Junctions in DUT
  if(abs(I_Forward) > abs(I_Reverse)) return FORWARD_JUNCTION;
  return REVERSE_JUNCTION;
}


void
Keithley236::onGpibCallback(int LocalUd, unsigned long LocalIbsta, unsigned long LocalIberr, long LocalIbcntl) {
  Q_UNUSED(LocalUd)
  Q_UNUSED(LocalIbsta)
  Q_UNUSED(LocalIberr)
  Q_UNUSED(LocalIbcntl)
}
