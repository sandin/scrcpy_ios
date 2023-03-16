#ifndef SCRCPY_IOS_USB_DEVICE_H
#define SCRCPY_IOS_USB_DEVICE_H

#include <functional>
#include <string>
#include <vector>

#include "lusb0_usb.h"

namespace scrcpy_ios {

class UsbDevice {
 public:
  UsbDevice(struct usb_device* dev = nullptr) : dev_(dev), dev_h_(nullptr) {}

  // disallow copy and assign
  UsbDevice(const UsbDevice&);
  void operator=(const UsbDevice&);

  // move constructor
  UsbDevice(UsbDevice&& other) : dev_(other.dev_), dev_h_(other.dev_h_) {
    other.dev_ = nullptr;
    other.dev_h_ = nullptr;
  }
  // move assignment operator
  UsbDevice& operator=(UsbDevice&& other) {
    dev_ = other.dev_;
    dev_h_ = other.dev_h_;
    other.dev_ = nullptr;
    other.dev_h_ = nullptr;
    return *this;
  }

  ~UsbDevice() {}

  bool Open();
  bool Close();

  int Control(int requesttype, int request, int value, int index, char* bytes, int size,
              int timeout) const;

  bool FindConfigDesc(unsigned char interface_class, unsigned char interface_sub_class) const;

  std::string GetName() const;

 private:
  struct usb_device* dev_;
  usb_dev_handle* dev_h_;
};

// Find all usb devices with a custom filter
using UsbDeviceFilter = std::function<bool(unsigned short, unsigned short)>;
UsbDevice* FindUsbDevice(UsbDeviceFilter filter);
void FreeUsbDevice(UsbDevice* device);

}  // namespace scrcpy_ios

#endif  // SCRCPY_IOS_USB_DEVICE_H