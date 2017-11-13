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
#ifndef LAKESHORE330_H
#define LAKESHORE330_H

#include <QObject>

class LakeShore330 : public QObject
{
  Q_OBJECT
public:
  explicit LakeShore330(int gpio, int address, QObject *parent = 0);
  virtual ~LakeShore330();
  int      init();
  void     onGpibCallback(int ud, unsigned long ibsta, unsigned long iberr, long ibcntl);
  double   getTemperature();
  bool     setTemperature(double Temperature);
  bool     switchPowerOn();
  bool     switchPowerOff();

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
  // Status Byte Register
  const quint8 SRQ;
  const quint8 ESB;
  const quint8 OVI;
  const quint8 CLE;
  const quint8 CDR;
  const quint8 SDR;
  // Standard Event Status Register
  const quint8 PON;
  const quint8 CME;
  const quint8 EXE;
  const quint8 DDE;
  const quint8 QYE;
  const quint8 OPC;
};

#endif // LAKESHORE330_H
