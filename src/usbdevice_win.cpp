﻿#ifdef LIBUSB_WIN32
#include <sstream>

#include "scrcpy_ios/macro_def.h"
#include "scrcpy_ios/usbdevice.h"

using namespace scrcpy_ios;

namespace scrcpy_ios {

static bool libusb_inited = false;

constexpr char ENDPOINT_NUM_MASK = 0x0f;
constexpr char ENDPOINT_DIRECTION_MASK = 0x80;

std::vector<std::unique_ptr<UsbDevice>> FindUsbDevices(UsbDeviceFilter filter);
if (!libusb_inited) {
  usb_init();
  libusb_inited = true;
}
usb_find_busses();  /* find all busses */
usb_find_devices(); /* find all connected devices */

std::vector<std::unique_ptr<UsbDevice>> devices;
struct usb_bus* bus = NULL;
struct usb_device* dev = NULL;
for (bus = usb_get_busses(); bus; bus = bus->next) {
  for (dev = bus->devices; dev; dev = dev->next) {
    if (filter(dev->descriptor.idVendor, dev->descriptor.idProduct)) {
      devices.emplace_back(std::make_shared<UsbDevice>(dev));
    }
  }
}

return devices;
}

void FreeUsbDevice(UsbDevice* device) { delete device; }

bool UsbDevice::FindInterface(unsigned char interface_class, unsigned char interface_sub_class,
                              UsbInterface& result) const {
  for (int i = 0; i < dev_->descriptor.bNumConfigurations; ++i) {
    struct usb_config_descriptor& config_desc = dev_->config[i];
    for (int l = 0; l < config_desc.bNumInterfaces; ++l) {
      struct usb_interface& usb_interface = config_desc.interface[l];
      for (int x = 0; x < usb_interface.num_altsetting; ++x) {
        struct usb_interface_descriptor& altsetting = usb_interface.altsetting[x];
        if (altsetting.bInterfaceClass == interface_class &&
            altsetting.bInterfaceSubClass == interface_sub_class) {
          result.config_desc_num = config_desc.bConfigurationValue;
          result.interface_num = altsetting.bInterfaceNumber;
          result.endpoints_size = altsetting.bNumEndpoints;
          for (int y = 0; y < altsetting.bNumEndpoints; ++y) {
            struct usb_endpoint_descriptor& endpoint = altsetting.endpoint[y];
            result.endpoints[y].address = endpoint.bEndpointAddress;
            result.endpoints[y].number = endpoint.bEndpointAddress & ENDPOINT_NUM_MASK;
            result.endpoints[y].direction =
                (endpoint.bEndpointAddress & ENDPOINT_DIRECTION_MASK) == USB_ENDPOINT_IN
                    ? UsbEndpointDirection::kIn
                    : UsbEndpointDirection::kOut;
          }
          return true;
        }
      }
    }
  }
  return false;  // NOT FOUND
}

int UsbDevice::SetConfiguration(unsigned char configuration_num) {
  ISCRCPY_ASSERT(dev_h_, "device handle can not be null!");
  return usb_set_configuration(dev_h_, configuration_num);
}

int UsbDevice::ClaimInterface(unsigned char interface_num) {
  ISCRCPY_ASSERT(dev_h_, "device handle can not be null!");
  return usb_claim_interface(dev_h_, interface_num);
}

int UsbDevice::ReleaseInterface(unsigned char interface_num) {
  ISCRCPY_ASSERT(dev_h_, "device handle can not be null!");
  return usb_release_interface(dev_h_, interface_num);
}

int UsbDevice::ClearHalt(unsigned char endpoint_address) {
  ISCRCPY_ASSERT(dev_h_, "device handle can not be null!");
  return usb_clear_halt(dev_h_, endpoint_address);
}

bool UsbDevice::Open() {
  dev_h_ = usb_open(dev_);
  return dev_h_ != nullptr;
}

bool UsbDevice::Reopen() {
  return false;  // TODO
}

bool UsbDevice::Close() {
  ISCRCPY_ASSERT(dev_h_, "device can not be null!");
  if (usb_close(dev_h_) == 0 /* OK */) {
    dev_h_ = nullptr;
    return true;
  }
  return false;
}

int UsbDevice::Control(int requesttype, int request, int value, int index, char* bytes, int size,
                       int timeout) const {
  ISCRCPY_ASSERT(dev_h_, "device handle can not be null!");
  return usb_control_msg(dev_h_, requesttype, request, value, index, bytes, size, timeout);
}

std::string UsbDevice::GetName() const {
  std::stringstream ss;
  ss << "VID_" << std::hex << dev_->descriptor.idVendor << "&PID_" << dev_->descriptor.idProduct;
  return ss.str();
}

std::string UsbDevice::GetSerialNumber() const {
  ISCRCPY_ASSERT(dev_h_, "device handle can not be null!");
  char buffer[256];
  usb_get_string_simple(dev_h_, dev_->descriptor.iSerialNumber, buffer, sizeof(buffer));
  return std::string(buffer);
}

}  // namespace scrcpy_ios

#endif  // LIBUSB_WIN32