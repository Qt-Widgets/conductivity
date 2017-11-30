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
#ifndef KEITHLEY236_H
#define KEITHLEY236_H

#include <QObject>
#include <QDateTime>
#ifdef Q_OS_LINUX
#include <QTimer>
#endif


class Keithley236 : public QObject
{
  Q_OBJECT

public:
  explicit Keithley236(int gpio, int address, QObject *parent = 0);
  virtual ~Keithley236();
  int      init();
  int      initVvsTSourceI(double dAppliedCurrent, double dCompliance);
  int      initVvsTSourceV(double dAppliedVoltage, double dCompliance);
  int      endVvsT();
  void     onGpibCallback(int ud, unsigned long ibsta, unsigned long iberr, long ibcntl);
  int      junctionCheck(double v1, double v2);
  bool     initISweep(double startCurrent, double stopCurrent, double currentStep, double delay);
  int      endISweep();
  bool     sendTrigger();
  bool     triggerSweep();
  bool     isReadyForTrigger();

signals:
  void     complianceEvent();
  void     readyForTrigger();
  void     newReading(QDateTime currentTime, QString sReading);
  void     sweepDone(QDateTime currentTime, QString sSweepData);

protected:

public slots:
#ifdef Q_OS_LINUX
  void checkNotify();

protected:
  QTimer pollTimer;
#endif

public:
  const int ERROR_JUNCTION;

  const int SRQ_DISABLED;
  const int WARNING;
  const int SWEEP_DONE;
  const int TRIGGER_OUT;
  const int READING_DONE;
  const int READY_FOR_TRIGGER;
  const int K236_ERROR;
  const int COMPLIANCE;

private:
  int gpibNumber;
  int k236Address;
  int k236;
  char spollByte;
  bool bStop;
  int iMask;
  int iComplianceEvents;
  QString sCommand;
  QString sResponse;
  double lastReading;
};

#endif // KEITHLEY236_H
