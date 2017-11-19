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

#include <ni4882.h>

#include <QDebug>
#include <QThread>
#include <QDateTime>

#define MAX_COMPLIANCE_EVENTS 5

namespace keithley236 {
  int rearmMask;
  int __stdcall
  myCallback(int LocalUd, unsigned long LocalIbsta, unsigned long LocalIberr, long LocalIbcntl, void* callbackData) {
    reinterpret_cast<Keithley236*>(callbackData)->onGpibCallback(LocalUd, LocalIbsta, LocalIberr, LocalIbcntl);
    return rearmMask;
  }
}


Keithley236::Keithley236(int gpio, int address, QObject *parent)
  : QObject(parent)
  //
  , ERROR_JUNCTION(-99999)
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
  , gpibNumber(gpio)
  , k236Address(address)
  , k236(-1)
{
}


Keithley236::~Keithley236() {
  if(k236 != -1) {
    ibnotify (k236, 0, NULL, NULL);// disable notification
    ibonl(k236, 0);// Disable hardware and software.
  }
}


int
Keithley236::init() {
  k236 = ibdev(gpibNumber, k236Address, 0, T3s, 1, 0);
  if(k236 < 0) {
    qDebug() << "ibdev() Failed";
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
    return GPIB_DEVICE_NOT_PRESENT;
  }
  short listen;
  ibln(k236, k236Address, NO_SAD, &listen);
  if(isGpibError("Keithley 236 Not Respondig"))
    return GPIB_DEVICE_NOT_PRESENT;
  if(listen == 0) {
    ibonl(k236, 0);
    qDebug() << "Nolistener at Addr";
    return GPIB_DEVICE_NOT_PRESENT;
  }
  // set up the asynchronous event notification routine on RQS
  ibnotify(k236,
           RQS,
           (GpibNotifyCallback_t) keithley236::myCallback,
           this);
  isGpibError("ibnotify call failed.");
  ibclr(k236);
  QThread::sleep(1);
  return 0;
}


int
Keithley236::initVvsT(double dAppliedCurrent, double dVoltageCompliance) {
  gpibWrite(k236, "M0,0X");     // SRQ Disabled, SRQ on Compliance
  gpibWrite(k236, "F1,0");      // Source I Measure V dc
  gpibWrite(k236, "O1");        // Remote Sense
  gpibWrite(k236, "P5");        // 32 Reading Filter
  gpibWrite(k236, "Z0");        // Disable Zero suppression
  gpibWrite(k236, "S3");        // 20ms integration time
  sCommand = QString("L%1,0").arg(dVoltageCompliance);
  gpibWrite(k236, sCommand);    // Set Compliance, Autorange Measure
  sCommand = QString("B%1,0,0").arg(dAppliedCurrent);
  gpibWrite(k236, sCommand);    // Set Applied Current
  gpibWrite(k236, "R0X");       // Disarm Trigger
  gpibWrite(k236, "T0,1,0,0X"); // Trigger on X ^SRC DLY MSR
  gpibWrite(k236, "R1");        // Arm Trigger
  gpibWrite(k236, "N1");        // Operate !
  gpibWrite(k236, "G5,2,0X");   // Output Source, Measure, No Prefix, DC
  // Give the instrument time to execute commands
  int srqMask =
      COMPLIANCE +
      K236_ERROR +
      READY_FOR_TRIGGER +
      READING_DONE +
      WARNING;
  sCommand = QString("M%1,0X").arg(srqMask);
  gpibWrite(k236, sCommand);   // SRQ Mask, Interrupt on Compliance
  if(isGpibError(QString("Keithley236::initVvsT: %1").arg(sCommand)))
    exit(-1);
  return 0;
}


int
Keithley236::endVvsT() {
  ibnotify (k236, 0, NULL, NULL);// disable notification
  gpibWrite(k236, "R0");         // Disarm Trigger
  gpibWrite(k236, "N0X");        // Place in Stand By
  gpibWrite(k236, "M0,0X");      // SRQ Disabled, SRQ on Compliance
  return 0;
}


// Returns the Order of Magnitude Difference
// between Forward and Reverse Current
int
Keithley236::junctionCheck() {
  uint iErr = 0;
  iErr |= gpibWrite(k236, "M0,0X");    // SRQ Disabled, SRQ on Compliance
  iErr |= gpibWrite(k236, "F0,0");     // Source V Measure I dc
  iErr |= gpibWrite(k236, "O1");       // Remote Sense
  iErr |= gpibWrite(k236, "P5");       // 32 Reading Filter
  iErr |= gpibWrite(k236, "Z0");       // Disable suppression
  iErr |= gpibWrite(k236, "S3");       // 20ms integration time
  iErr |= gpibWrite(k236, "R0X");      // Disarm Trigger
  iErr |= gpibWrite(k236, "T0,1,0,0"); // Trigger on X ^SRC DLY MSR
  iErr |= gpibWrite(k236, "L1.0e-4,0");// Set Compliance, Autorange Measure
  iErr |= gpibWrite(k236, "G5,2,0");   // Output Source, Measure, No Prefix, DC
  iErr |= gpibWrite(k236, "B1.0,0,0"); // Source 1.0V Measure I Autorange
  iErr |= gpibWrite(k236, "R1");       // Arm Trigger
  iErr |= gpibWrite(k236, "N1X");      // Operate !
  if(iErr & ERR) {
    QString sError;
    sError = QString("Keithley236::junctionCheck(): GPIB Error in gpibWrite(): - Status= %1")
                     .arg(ThreadIbsta(), 4, 16, QChar('0'));
    qCritical() <<  sError;
    sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qCritical() << sError;
    return ERROR_JUNCTION;
  }
  // Rischio di loop infinito
  ibrsp(k236, &spollByte);
  while(!(spollByte & 16)) {// Ready for trigger
    ibrsp(k236, &spollByte);
    QThread::msleep(100);
  }
  // Get the forward current value
  gpibWrite(k236, "H0X");
  if(isGpibError("Keithley236::junctionCheck(): Trigger Error"))
    return ERROR_JUNCTION;
  // Rischio di loop infinito
  ibrsp(k236, &spollByte);
  while(!(spollByte & 8)) {// Reading Done
    ibrsp(k236, &spollByte);
    QThread::msleep(100);
  }
  double I_Forward = gpibRead(k236).toDouble();
  qInfo() << QString("Forward Current [A]= ") + sCommand;
  gpibWrite(k236, "B-1.0,0,0X"); // Source -1.0V Measure I Autorange
  if(isGpibError("Keithley236::junctionCheck(): Error Changing Output Voltage"))
    return ERROR_JUNCTION;
  QThread::sleep(1);
  double I_Reverse = gpibRead(k236).toDouble();
  qDebug() << QString("Reverse Current [A]= ") + sCommand;
  gpibWrite(k236, "B0.0,0,0"); // Source 0.0V Measure I Autorange
  if(isGpibError("Keithley236::junctionCheck(): Zeroing Output Voltage"))
    return ERROR_JUNCTION;
  gpibWrite(k236, "N0X");      // Stand By !
  if(isGpibError("Keithley236::junctionCheck(): Placing Keithely 236 in Standby Mode"))
    return ERROR_JUNCTION;
  if((I_Forward == 0.0) || (I_Reverse == 0.0))
    return ERROR_JUNCTION;
  return qRound(log10(abs(I_Forward))) -
         qRound(log10(abs(I_Reverse)));
}


bool
Keithley236::initISweep(double startCurrent, double stopCurrent, double currentStep, double delay) {
  uint iErr = 0;
  iErr |= gpibWrite(k236, "M0,0X");    // SRQ Disabled, SRQ on Compliance
  iErr |= gpibWrite(k236, "F1,1");     // Source I, Sweep mode
  iErr |= gpibWrite(k236, "O1");       // Remote Sense
  iErr |= gpibWrite(k236, "T0,1,0,0"); // Trigger on X ^SRC DLY MSR
  iErr |= gpibWrite(k236, "L10.0,0");  // 10V Compliance, Autorange Measure
  iErr |= gpibWrite(k236, "G5,2,2");   // Output Source and Measure, No Prefix, All Lines Sweep Data
  sCommand = QString("M%1,0").arg(SWEEP_DONE);// SRQ On Sweep Done
  iErr |= gpibWrite(k236, sCommand);

  // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

  iErr |= gpibWrite(k236, "B0,0,0");   // Source V Measure I dc
  iErr |= gpibWrite(k236, "Z0");       // Disable suppression
  sCommand = QString("Q1,%1,%2,%3,0,%4")
            .arg(startCurrent)
            .arg(stopCurrent)
            .arg(currentStep)
            .arg(delay);
  iErr |= gpibWrite(k236, sCommand);   // Program Sweep
  iErr |= gpibWrite(k236, "R1");       // Arm Trigger
  iErr |= gpibWrite(k236, "N1X");      // Operate !
  if(iErr & ERR) {
    QString sError;
    sError = QString("Keithley236::initISweep(): GPIB Error in gpibWrite(): - Status= %1")
                     .arg(ThreadIbsta(), 4, 16, QChar('0'));
    qCritical() <<  sError;
    sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qCritical() << sError;
    return false;
  }
  // Rischio di loop infinito
  do {
    QThread::sleep(100);
    ibrsp(k236, &spollByte);
  } while (!(spollByte & READY_FOR_TRIGGER));
  gpibWrite(k236, "H0X");
  if(isGpibError("Keithley236::initISweep(): Trigger Error"))
    return false;
  return true;
}


void
Keithley236::onGpibCallback(int LocalUd, unsigned long LocalIbsta, unsigned long LocalIberr, long LocalIbcntl) {
  Q_UNUSED(LocalIbsta)
  Q_UNUSED(LocalIberr)
  Q_UNUSED(LocalIbcntl)

  if(ibrsp(LocalUd, &spollByte) & ERR) {
    qCritical() << QString("GPIB error %1").arg(LocalIberr);
  }

  if(spollByte & WARNING) {// Warning
    gpibWrite(LocalUd, "U9X");
    sCommand = gpibRead(LocalUd);
    qCritical() << "Keithley236::onGpibCallback: Warning" << sCommand;
  }

  if(spollByte & SWEEP_DONE) {// Sweep Done
    QDateTime currentTime = QDateTime::currentDateTime();
    emit sweepDone(currentTime, gpibRead(LocalUd));
  }

  if(spollByte & TRIGGER_OUT) {// Trigger Out
    qCritical() << "Keithley236::onGpibCallback: Trigger Out ?";
  }

  if(spollByte & COMPLIANCE) {// Compliance
    iComplianceEvents++;
    qCritical() << QString("Keithley236::onGpibCallback: ComplianceEvents[%21]: Last Value= %2")
                   .arg(iComplianceEvents)
                   .arg(lastReading);
    QThread::msleep(300);
    if(iComplianceEvents > MAX_COMPLIANCE_EVENTS) {
      qCritical() << QString("Keithley236::onGpibCallback:  Measure Stopped");
      emit complianceEvent();
    }
  }

  if(spollByte & K236_ERROR) {// Error
    gpibWrite(LocalUd, "U1X");
    sCommand = gpibRead(LocalUd);
    qCritical() << "Keithley236::onGpibCallback: Error" << sCommand;
  }

  if(spollByte & READY_FOR_TRIGGER) {// Ready for trigger
    emit readyForTrigger();
  }

  if(spollByte & READING_DONE) {// Reading Done
    QDateTime currentTime = QDateTime::currentDateTime();
    sResponse = gpibRead(LocalUd);
    emit newReading(currentTime, sResponse);
  }

  keithley236::rearmMask = RQS;
}

bool
Keithley236::sendTrigger() {
  gpibWrite(k236, "H0X");
  if(isGpibError("Keithley236::sendTrigger(): Trigger Error"))
    return false;
  return true;
}
