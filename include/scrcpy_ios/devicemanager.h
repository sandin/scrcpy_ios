#ifndef SCRCPY_IOS_DEVICE_MANAGER_H
#define SCRCPY_IOS_DEVICE_MANAGER_H

#include <memory>  // unique_ptr
#include <string>
#include <vector>

#include "scrcpy_ios/usbdevice.h"

namespace scrcpy_ios {

class DeviceManager {
 public:
  using UsbDeviceList = std::vector<std::unique_ptr<UsbDevice>>;

  static std::unique_ptr<UsbDevice> FindIosDevice(std::string usb_serial);
  static UsbDeviceList FindAllIosDevices();
};

}  // namespace scrcpy_ios

#endif  // SCRCPY_IOS_DEVICE_MANAGER_H