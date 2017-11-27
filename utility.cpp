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
#include "utility.h"
#include <QDebug>


namespace gpibUtilities {
char readBuf[2001];
}


#ifdef Q_OS_LINUX
int
ibnotify(int boardOrDevice, int eventMask, GpibNotifyCallback_t callbackFunction, void *callbackData) {
  Q_UNUSED(boardOrDevice)
  Q_UNUSED(eventMask)
  Q_UNUSED(callbackFunction)
  Q_UNUSED(callbackData)
  return 0;
}
#endif


bool
isGpibError(QString sErrorString) {
  if(ThreadIbsta() & ERR) {
    qCritical() << sErrorString;
    QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcnt());
    qCritical() << sError;
    return true;
  }
  return false;
}


QString
ErrMsg(int sta, int err, long cntl) {
  QString sError, sTmp;

  sError = QString("status = 0x%1 <").arg(sta, 4, 16, QChar('0'));
  if (sta & ERR )  sError += QString(" ERR");
  if (sta & TIMO)  sError += QString(" TIMO");
  if (sta & END )  sError += QString(" END");
  if (sta & SRQI)  sError += QString(" SRQI");
  if (sta & RQS )  sError += QString(" RQS");
  if (sta & CMPL)  sError += QString(" CMPL");
  if (sta & LOK )  sError += QString(" LOK");
  if (sta & REM )  sError += QString(" REM");
  if (sta & CIC )  sError += QString(" CIC");
  if (sta & ATN )  sError += QString(" ATN");
  if (sta & TACS)  sError += QString(" TACS");
  if (sta & LACS)  sError += QString(" LACS");
  if (sta & DTAS)  sError += QString(" DTAS");
  if (sta & DCAS)  sError += QString(" DCAS");

  sTmp = QString("> error = 0x%1").arg(err, 4, 16, QChar('0'));
  sError += sTmp;
  if (err == EDVR) sError += QString(" EDVR <DOS Error>");
  if (err == ECIC) sError += QString(" ECIC <Not CIC>");
  if (err == ENOL) sError += QString(" ENOL <No Listener>");
  if (err == EADR) sError += QString(" EADR <Address error>");
  if (err == EARG) sError += QString(" EARG <Invalid argument>");
  if (err == ESAC) sError += QString(" ESAC <Not Sys Ctrlr>");
  if (err == EABO) sError += QString(" EABO <Op. aborted>");
  if (err == ENEB) sError += QString(" ENEB <No GPIB board>");
  if (err == EOIP) sError += QString(" EOIP <Async I/O in prg>");
  if (err == ECAP) sError += QString(" ECAP <No capability>");
  if (err == EFSO) sError += QString(" EFSO <File sys. error>");
  if (err == EBUS) sError += QString(" EBUS <Command error>");
  if (err == ESTB) sError += QString(" ESTB <Status byte lost>");
  if (err == ESRQ) sError += QString(" ESRQ <SRQ stuck on>");
  if (err == ETAB) sError += QString(" ETAB <Table Overflow>");

  sTmp = QString(" count = 0x%1").arg(cntl, 4, 16, QChar('0'));
  sError += sTmp;
  return sError;
}


uint
gpibWrite(int ud, QString sCmd) {
//  qDebug() << QString("Writing %1 bytes of data: Data = %2").arg(sCmd.length()).arg(sCmd.toUtf8().constData());
  ibwrt(ud, sCmd.toUtf8().constData(), sCmd.length());
  isGpibError("GPIB Writing Error Writing");
  return ThreadIbsta();
}


QString
gpibRead(int ud) {
  QString sString;
  do {
    ibrd(ud, gpibUtilities::readBuf, sizeof(gpibUtilities::readBuf)-1);
    if(isGpibError("GPIB Reading Error"))
      return sString;
    gpibUtilities::readBuf[ThreadIbcnt()] = 0;
    sString += QString(gpibUtilities::readBuf);
  } while(ThreadIbcnt() == sizeof(gpibUtilities::readBuf)-1);
  return sString;
}

