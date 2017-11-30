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
#ifndef UTILITY_H
#define UTILITY_H

#include <QString>
#ifdef Q_OS_LINUX
#include <gpib/ib.h>
#include <QObject>
#include <QTimer>
#else
#include <ni4882.h>
#endif

#define GPIB_DEVICE_NOT_PRESENT -1000
#define NO_ERROR 0

QString ErrMsg(int sta, int err, long cntl);
QString gpibRead(int ud);
uint    gpibWrite(int ud, QString sCmd);
bool    isGpibError(QString sErrorString);

#endif // UTILITY_H
