#include "utility.h"
#include <QDebug>

char readBuf[2001];

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


int
gpibWrite(int ud, QString sCmd) {
  int iRes = ibwrt(ud, sCmd.toUtf8().constData(), sCmd.length());
  if(iRes & ERR) {
    QString sError;
    sError = QString("GPIB Error Writing: %1 - Status= %2").arg(sCmd).arg(ThreadIbsta(), 4, 16, QChar('0'));
    qDebug() <<  sError;
    sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() <<  sError;
  }
  return iRes;
}


QString
gpibRead(int ud) {
  QString sString;
  if(ibrd(ud, readBuf, sizeof(readBuf)-1) & ERR) {
    QString sError;
    sError = QString("GPIB Reading Error - Status= %1").arg(ThreadIbsta(), 4, 16, QChar('0'));
    qDebug() << sError;
    sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
    qDebug() << sError;
  }
  readBuf[ThreadIbcntl()] = 0;
  sString = QString(readBuf);
  return sString;
}

