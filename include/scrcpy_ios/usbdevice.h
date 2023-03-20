#ifndef SCRCPY_IOS_USB_DEVICE_H
#define SCRCPY_IOS_USB_DEVICE_H

#include <functional>
#include <memory>
#include <string>
#include <vector>

#ifdef LIBUSB_WIN32
#include "lusb0_usb.h"
#endif

namespace scrcpy_ios {

enum class UsbEndpointDirection { kOut, kIn };

struct UsbEndpoint {
  unsigned char address;
  unsigned char number;
  UsbEndpointDirection direction;
};

struct UsbInterface {
  unsigned char config_desc_num;
  unsigned char interface_num;
  UsbEndpoint endpoints[32];
  unsigned char endpoints_size;
};

class UsbDevice {
 public:
  enum Result { Ok = 0 };

  UsbDevice(struct usb_device* dev) : dev_(dev), dev_h_(nullptr) {}

  ~UsbDevice() {}

  bool Open();
  bool Reopen();
  bool Close();

  bool IsOpened() const;

  int Control(int requesttype, int request, int value, int index, char* bytes, int size,
              int timeout) const;

  bool FindInterface(unsigned char interface_class, unsigned char interface_sub_class,
                     UsbInterface& result) const;
  int SetConfiguration(unsigned char configuration_num);
  int ReleaseInterface(unsigned char interface_num);
  int ClaimInterface(unsigned char interface_num);
  int ClearHalt(unsigned char endpoint_address);

  std::string GetName() const;
  std::string GetSerialNumber() const;
  unsigned short GetVendorId() const;
  unsigned short GetProductId() const;

 private:
#ifdef LIBUSB_WIN32
  struct usb_device* dev_;
  usb_dev_handle* dev_h_;
#endif
};

// Find all usb devices with a custom filter
using UsbDeviceFilter = std::function<bool(UsbDevice*)>;
std::vector<std::unique_ptr<UsbDevice>> FindUsbDevices(UsbDeviceFilter filter);

}  // namespace scrcpy_ios

#endif  // SCRCPY_IOS_USB_DEVICE_H