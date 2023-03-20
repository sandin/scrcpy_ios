#ifndef SCRCPY_IOS_DEVICE_MANAGER_H
#define SCRCPY_IOS_DEVICE_MANAGER_H

#include <memory>  // unique_ptr
#include <string>
#include <vector>

#include "scrcpy_ios/usbdevice.h"

namespace scrcpy_ios {

constexpr int kUSBAppleVendorID = 0x05AC;

constexpr int kVendorSpecInterfaceclass = 0xFF;
constexpr int kUsbmuxInterfaceClass = 0xFE;
constexpr int kQuicktimeInterfaceClass = 0x2A;

class DeviceManager {
 public:
  using UsbDeviceList = std::vector<std::unique_ptr<UsbDevice>>;

  static std::unique_ptr<UsbDevice> FindIosDevice(std::string usb_serial);
  static UsbDeviceList FindAllIosDevices();
};

}  // namespace scrcpy_ios

#endif  // SCRCPY_IOS_DEVICE_MANAGER_H