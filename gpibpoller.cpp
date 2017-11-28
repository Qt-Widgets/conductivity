#include "gpibpoller.h"
#include "utility.h"
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
  pollTimer.start(10);
}


void
GpibPoller::endPolling() {
  pollTimer.stop();
  disconnect(&pollTimer, 0, 0, 0);
}


void
GpibPoller::poll() {
  char spollByte;
  ibrsp(ud, &spollByte);
  if(!(spollByte & 64))
    return; // SRQ not enabled
  emit gpibNotify(ud, ThreadIbsta(), ThreadIberr(), ThreadIbcnt());
}
