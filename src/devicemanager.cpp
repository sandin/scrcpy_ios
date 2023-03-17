#include "scrcpy_ios/devicemanager.h"

#include "scrcpy_ios/macro_def.h"
#include "scrcpy_ios/usbdevice.h"

using namespace scrcpy_ios;

constexpr int kiPhoneVendorId = 0x05AC;
constexpr int kiPhoneMaxProductId = 0x12A8;

// static
std::unique_ptr<UsbDevice> DeviceManager::FindIosDevice(std::string usb_serial) {
  return nullptr;  // TODO:
}

// static
DeviceManager::UsbDeviceList DeviceManager::FindAllIosDevices() {
  return FindUsbDevices([](unsigned short vid, unsigned short pid) {
    return vid == kiPhoneVendorId && pid <= kiPhoneMaxProductId;
  });
}
