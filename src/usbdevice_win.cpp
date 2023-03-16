#ifdef LIBUSB_WIN32
#include <assert.h>

#include <sstream>

#include "scrcpy_ios/usbdevice.h"

using namespace scrcpy_ios;

namespace scrcpy_ios {

UsbDevice* FindUsbDevice(UsbDeviceFilter filter) {
  static bool inited = false;
  if (!inited) {
    usb_init();
  }

  usb_find_busses();  /* find all busses */
  usb_find_devices(); /* find all connected devices */

  struct usb_bus* bus = NULL;
  struct usb_device* dev = NULL;
  for (bus = usb_get_busses(); bus; bus = bus->next) {
    for (dev = bus->devices; dev; dev = dev->next) {
      if (filter(dev->descriptor.idVendor, dev->descriptor.idProduct)) {
        return new UsbDevice(dev);
      }
    }
  }

  return nullptr;
}

void FreeUsbDevice(UsbDevice* device) { free(device); }

bool UsbDevice::FindConfigDesc(unsigned char interface_class,
                               unsigned char interface_sub_class) const {
  for (int i = 0; i < dev_->descriptor.bNumConfigurations; ++i) {
    struct usb_config_descriptor& config_desc = dev_->config[i];
    for (int l = 0; l < config_desc.bNumInterfaces; ++l) {
      struct usb_interface& usb_interface = config_desc.interface[l];
      for (int x = 0; x < usb_interface.num_altsetting; ++x) {
        struct usb_interface_descriptor& altsetting = usb_interface.altsetting[x];
        if (altsetting.bInterfaceClass == interface_class &&
            altsetting.bInterfaceSubClass == interface_sub_class) {
          return true;
        }
      }
    }
  }
  return false;
}

bool UsbDevice::Open() {
  dev_h_ = usb_open(dev_);
  return dev_h_ != nullptr;
}

bool UsbDevice::Close() {
  assert(dev_h_);
  return usb_close(dev_h_) == 0 /* OK */;
}

int UsbDevice::Control(int requesttype, int request, int value, int index, char* bytes, int size,
                       int timeout) const {
  assert(dev_h_);
  return usb_control_msg(dev_h_, requesttype, request, value, index, bytes, size, timeout);
}

std::string UsbDevice::GetName() const {
  std::stringstream ss;
  ss << "VID_" << std::hex << dev_->descriptor.idVendor << "&PID_" << dev_->descriptor.idProduct;
  return ss.str();
}

}  // namespace scrcpy_ios

#endif  // LIBUSB_WIN32