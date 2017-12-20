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
    cs130 = ibdev(gpibNumber, cs130Address, 0, T10s, 1, 0);
    if(cs130 < 0) {
        qDebug() << "ibdev() Failed";
        QString sError = ErrMsg(ThreadIbsta(), ThreadIberr(), ThreadIbcntl());
        qDebug() << sError;
        return GPIB_DEVICE_NOT_PRESENT;
    }
    short listen;
    ibln(gpibNumber, cs130Address, NO_SAD, &listen);
    if(isGpibError("CornerStone 130 Not Respondig"))
        return GPIB_DEVICE_NOT_PRESENT;
    if(listen == 0) {
        ibonl(cs130, 0);
        qDebug() << "Nolistener at Addr";
        return GPIB_DEVICE_NOT_PRESENT;
    }
    ibclr(cs130);
    QThread::sleep(1);
    return NO_ERROR;
}


