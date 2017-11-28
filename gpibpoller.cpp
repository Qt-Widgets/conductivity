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
  pollTimer.stop();
  disconnect(&pollTimer, 0, 0, 0);
}


void
GpibPoller::poll() {
  ibwait(ud, 0);// To update the gpib status
  qInfo() << QString("ibsta = 0x%1, mask = 0x%2")
             .arg(ThreadIbsta(), 4, 16)
             .arg(mask, 4, 16);
  if((ThreadIbsta() & mask) != 0) {
    qDebug() << QString("GPIB Event: 0x%1")
                .arg(ThreadIbsta() & mask, 4, 16);
    emit gpibNotify(ud, ThreadIbsta(), ThreadIberr(), ThreadIbcnt());
  }
}
