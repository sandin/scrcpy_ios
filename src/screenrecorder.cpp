#include "scrcpy_ios/screenrecorder.h"

#include "scrcpy_ios/macro_def.h"

using namespace scrcpy_ios;

// static
bool ScreenRecorder::EnableQuicktimeConfigDesc(UsbDevice* dev) {
  ISCRCPY_ASSERT(dev, "device can not be null!");
  return dev->Control(0x40, 0x52, 0x00, 0x02, NULL, 0, 1000) == UsbDevice::Ok;
}

// static
bool ScreenRecorder::DisableQuicktimeConfigDesc(UsbDevice* dev) {
  ISCRCPY_ASSERT(dev, "device can not be null!");
  return dev->Control(0x40, 0x52, 0x00, 0x00, NULL, 0, 1000) == UsbDevice::Ok;
}

// static
bool ScreenRecorder::ClearFeature(UsbDevice* dev, unsigned char endpoint_address) {
  ISCRCPY_ASSERT(dev, "device can not be null!");
  return dev->Control(0x02, 0x01, 0x00, endpoint_address, NULL, 0, 1000) == UsbDevice::Ok;
}
