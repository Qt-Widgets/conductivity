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
    cs130 = ibdev(gpibNumber, cs130Address, 0, T30s, 1, REOS|0x000A);
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

    // Abort any operation in progress
    gpibWrite(cs130, "ABORT\r\n");
    if(isGpibError("CornerStone 130 ABORT Failed"))
        return GPIB_DEVICE_NOT_PRESENT;

    closeShutter();

    // We express the Wavelengths in nm
    gpibWrite(cs130, "UNITS NM\r\n");
    if(isGpibError("CornerStone 130 UNITS Failed"))
        return GPIB_DEVICE_NOT_PRESENT;
    // Set initial Filter Wavelength
    if(!setGrating(1))
        return GPIB_DEVICE_NOT_PRESENT;
    if(!setWavelength(560.0))
        return GPIB_DEVICE_NOT_PRESENT;
    return NO_ERROR;
}


bool
CornerStone130::openShutter() {
    gpibWrite(cs130, "SHUTTER O\r\n");
    if(isGpibError("CornerStone 130 Open SHUTTER Failed"))
        return false;
    return true;
}


bool
CornerStone130::closeShutter() {
    gpibWrite(cs130, "SHUTTER C\r\n");
    if(isGpibError("CornerStone 130 Close SHUTTER Failed"))
        return false;
    return true;
}


bool
CornerStone130::setWavelength(double waveLength) {
    sCommand = QString("GOWAVE %1\r\n").arg(waveLength);
    gpibWrite(cs130, sCommand);
    if(isGpibError("CornerStone 130 GOWAVE Failed"))
        return false;
    QThread::sleep(1);
    gpibWrite(cs130, "WAVE?\r\n");
    if(isGpibError("CornerStone 130 WAVE? Failed"))
        return false;
    sResponse = gpibRead(cs130);
    if(isGpibError("CornerStone 130 WAVE? readback Failed"))
        return false;
    qDebug() << "Present Wavelength =" << sResponse << "nm";
    return true;
}


bool
CornerStone130::setGrating(int grating) {
    if(grating <1 || grating > 2)
        return false;
    sCommand = QString("GRAT %1\r\n").arg(grating);
    gpibWrite(cs130, sCommand);
    if(isGpibError("CornerStone 130 GRAT Failed"))
        return false;
    return true;
}
