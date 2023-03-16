#ifndef SCRCPY_IOS_SCREEN_RECORDER_H
#define SCRCPY_IOS_SCREEN_RECORDER_H

#include "scrcpy_ios/usbdevice.h"

namespace scrcpy_ios {

class ScreenRecorder {
 public:
  ScreenRecorder(UsbDevice* dev) : dev_(dev) {}

  ~ScreenRecorder() { FreeUsbDevice(dev_); }

  static bool EnableQuicktimeConfigDesc(UsbDevice* dev);

 private:
  UsbDevice* dev_;
};

}  // namespace scrcpy_ios

#endif  // SCRCPY_IOS_SCREEN_RECORDER_H