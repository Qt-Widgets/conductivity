#include "cornerstone130.h"
#include "utility.h"

#if defined(Q_OS_LINUX)
#include <gpib/ib.h>
#else
#include <ni4882.h>
#endif

#include <QDebug>
#include <QThread>


CornerStone130::CornerStone130(int gpio, int address, QObject *parent)
    : QObject(parent)
    , gpibNumber(gpio)
    , cs130Address(address)
    , cs130(-1)
{
}


CornerStone130::~CornerStone130() {
    if(cs130 != -1) {
        ibonl(cs130, 0);// Disable hardware and software.
    }
}


int
CornerStone130::init() {
    cs130 = ibdev(gpibNumber, cs130Address, 0, T30s, 1, 0);
    if(cs130 < 0) {
        qDebug() << "ibdev() Failed";
        QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
        qDebug() << sError;
        return GPIB_DEVICE_NOT_PRESENT;
    }
    ibclr(cs130);
    if(isGpibError("CornerStone 130 Not Respondig"))
        return GPIB_DEVICE_NOT_PRESENT;
    QThread::sleep(1);
    gpibWrite(cs130, "ABORT\r\n");      // SRQ Disabled, SRQ on Compliance
    if(isGpibError("CornerStone 130 ABORT Failed"))
        return GPIB_DEVICE_NOT_PRESENT;
    return NO_ERROR;
}


