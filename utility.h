#ifndef UTILITY_H
#define UTILITY_H

#include <QString>
#include <ni4882.h>

#define GPIB_DEVICE_NOT_PRESENT -1000

QString ErrMsg(int sta, int err, long cntl);
QString gpibRead(int ud);
int     gpibWrite(int ud, QString sCmd);

#endif // UTILITY_H
