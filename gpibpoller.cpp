#include "gpibpoller.h"
#include <QDebug>

GpibPoller::GpibPoller(int device, QObject *parent)
  : QObject(parent)
  , ud(device)
  , mask(0)
{
}


void
GpibPoller::startPolling(int eventMask) {
  mask = eventMask;
  connect(&pollTimer, SIGNAL(timeout()),
          this, SLOT(poll()));
  pollTimer.start(1000);
  qInfo() << "GpibPoller::startPolling";
}


void
GpibPoller::endPolling() {
}


void
GpibPoller::poll() {
  ibwait(ud, 0);
  qInfo() << QString("ibsta= %1, mask = %2").arg(ThreadIbsta()).arg(mask);
  if((ThreadIbsta() & mask) != 0)
    emit gpibNotify(ud, ThreadIbsta(), ThreadIberr(), ThreadIbcnt());
}
