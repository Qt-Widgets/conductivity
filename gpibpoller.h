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
#ifndef GPIBPOLLER_H
#define GPIBPOLLER_H

#include <QObject>
#include <QTimer>

#ifdef Q_OS_LINUX
#include <gpib/ib.h>
#else
#include <ni4882.h>
#endif

class GpibPoller : public QObject
{
    Q_OBJECT
public:
  explicit GpibPoller(int device, QObject *parent = 0);
  void startPolling(int eventMask);
  void endPolling();

public slots:
  void poll();

signals:
  void gpibNotify(int ud, unsigned long ibsta, unsigned long iberr, long ibcntl);

protected:
  int ud;
  int mask;
  QTimer pollTimer;
};
#endif // GPIBPOLLER_H
