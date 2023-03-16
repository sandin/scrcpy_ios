#include "scrcpy_ios/screenrecorder.h"

#include <assert.h>

using namespace scrcpy_ios;

// static
bool ScreenRecorder::EnableQuicktimeConfigDesc(UsbDevice* dev) {
  assert(dev);
  return dev->Control(0x40, 0x52, 0x00, 0x02, nullptr, 0, 1000) == 0 /* OK */;
}
