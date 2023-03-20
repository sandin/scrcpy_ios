#include "scrcpy_ios/devicemanager.h"

#include "scrcpy_ios/macro_def.h"
#include "scrcpy_ios/usbdevice.h"

using namespace scrcpy_ios;

// static
std::unique_ptr<UsbDevice> DeviceManager::FindIosDevice(std::string usb_serial) {
  std::vector<std::unique_ptr<UsbDevice>> devices = FindUsbDevices([&](UsbDevice* dev) {
    if (dev->GetVendorId() == kUSBAppleVendorID) {
      UsbInterface usbmux_interface_;
      bool has_usbmux =
          dev->FindInterface(kVendorSpecInterfaceclass, kUsbmuxInterfaceClass, usbmux_interface_);
      if (has_usbmux) {
        dev->Open();
        bool match = dev->GetSerialNumber() == usb_serial;
        dev->Close();
        return match;
      }
    }
    return false;
  });
  return devices.size() > 0 ? std::move(devices.at(0)) : nullptr;  // TODO:
}

// static
DeviceManager::UsbDeviceList DeviceManager::FindAllIosDevices() {
  return FindUsbDevices([](UsbDevice* dev) {
    if (dev->GetVendorId() == kUSBAppleVendorID) {
      UsbInterface usbmux_interface_;
      return dev->FindInterface(kVendorSpecInterfaceclass, kUsbmuxInterfaceClass,
                                usbmux_interface_);
    }
    return false;
  });
}
