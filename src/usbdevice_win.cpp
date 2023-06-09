﻿#ifdef LIBUSB_WIN32
#include <sstream>

#include "scrcpy_ios/macro_def.h"
#include "scrcpy_ios/usbdevice.h"
#include "scrcpy_ios/utils.h"

using namespace scrcpy_ios;

namespace scrcpy_ios {

static bool libusb_inited = false;

constexpr char ENDPOINT_NUM_MASK = 0x0f;
constexpr char ENDPOINT_DIRECTION_MASK = 0x80;

using UsbDeviceWalker = std::function<void(struct usb_bus*, struct usb_device*)>;
static void ForEachUsbDevices(UsbDeviceWalker walker) {
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
      walker(bus, dev);
    }
  }
}

static struct usb_device* FindOneUsbDevice(unsigned short vendor_id, unsigned short product_id,
                                           std::string serial_number) {
  struct usb_device* found = nullptr;
  ForEachUsbDevices([&](struct usb_bus* bus, struct usb_device* dev) {
    if (dev->descriptor.idVendor == vendor_id && dev->descriptor.idProduct == product_id) {
      usb_dev_handle* dev_h = usb_open(dev);
      char buffer[256];
      usb_get_string_simple(dev_h, dev->descriptor.iSerialNumber, buffer, sizeof(buffer));
      bool match = strcmp(serial_number.c_str(), buffer) == 0;
      usb_close(dev_h);
      found = dev;
    }
  });
  return found;
}

std::vector<std::unique_ptr<UsbDevice>> FindUsbDevices(UsbDeviceFilter filter) {
  std::vector<std::unique_ptr<UsbDevice>> devices;
  ForEachUsbDevices([&](struct usb_bus* bus, struct usb_device* dev) {
    std::unique_ptr<UsbDevice> device = std::make_unique<UsbDevice>(dev);
    if (filter(device.get())) {
      devices.emplace_back(std::move(device));
    }
  });
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
            ISCRCPY_LOG_V("Found endpoint bEndpointAddress=%d, wMaxPacketSize=%d\n",
                          endpoint.bEndpointAddress, endpoint.wMaxPacketSize);
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
  int ret = usb_set_configuration(dev_h_, configuration_num);
  ISCRCPY_LOG_V("usb_set_configuration, dev=%p, configuration_num=%d, ret=%d\n", dev_h_,
                configuration_num, ret);
  return ret;
}

int UsbDevice::ClaimInterface(unsigned char interface_num) {
  ISCRCPY_ASSERT(dev_h_, "device handle can not be null!");
  int ret = usb_claim_interface(dev_h_, interface_num);
  ISCRCPY_LOG_V("usb_claim_interface, dev=%p, interface_num=%d, ret=%d\n", dev_h_, interface_num,
                ret);
  return ret;
}

int UsbDevice::ReleaseInterface(unsigned char interface_num) {
  ISCRCPY_ASSERT(dev_h_, "device handle can not be null!");
  int ret = usb_release_interface(dev_h_, interface_num);
  ISCRCPY_LOG_V("usb_release_interface, dev=%p, interface_num=%d, ret=%d\n", dev_h_, interface_num,
                ret);
  return ret;
}

int UsbDevice::ClearHalt(unsigned char endpoint_address) {
  ISCRCPY_ASSERT(dev_h_, "device handle can not be null!");
  int ret = usb_clear_halt(dev_h_, endpoint_address);
  ISCRCPY_LOG_V("usb_clear_halt, dev=%p, endpoint_address=0x%x, ret=%d\n", dev_h_, endpoint_address,
                ret);
  return ret;
}

bool UsbDevice::Open() {
  dev_h_ = usb_open(dev_);
  ISCRCPY_LOG_V("usb_open, dev=%p, ret=%d\n", dev_, dev_h_);
  return dev_h_ != nullptr;
}

bool UsbDevice::IsOpened() const { return dev_h_ != nullptr; }

bool UsbDevice::Reopen() {
  ISCRCPY_ASSERT(IsOpened(), "The device should have been opened.");

  std::string serial_number = GetSerialNumber();
  unsigned short vendor_id = GetVendorId();
  unsigned short product_id = GetProductId();
  Close();

  Sleep(30 * 1000);  // TODO: windows API

  // Find and create a new device with same vendor_id & product_id & serial_number
  struct usb_device* dev = FindOneUsbDevice(vendor_id, product_id, serial_number);
  if (!dev) {
    ISCRCPY_LOG_E("Can not find the device, vendor_id=0x%x, product_id=0x%x\n", vendor_id,
                  product_id);
    return false;
  }
  dev_ = dev;
  return Open();
}

bool UsbDevice::Close() {
  ISCRCPY_ASSERT(dev_h_, "device can not be null!");
  int ret = usb_close(dev_h_);
  ISCRCPY_LOG_V("usb_close, dev=%p, ret=%d\n", dev_h_, ret);
  if (ret == 0 /* OK */) {
    dev_h_ = nullptr;
    return true;
  }
  return false;
}

int UsbDevice::Control(int requesttype, int request, int value, int index, char* bytes, int size,
                       int timeout) const {
  ISCRCPY_ASSERT(dev_h_, "device handle can not be null!");
  int ret = usb_control_msg(dev_h_, requesttype, request, value, index, bytes, size, timeout);
  ISCRCPY_LOG_V(
      "usb_control_msg, dev=%p, requesttype=%d, request=%d, value=%d, index=%d, bytes=%p, size=%d, "
      "timeout=%d, ret=%d\n",
      dev_h_, requesttype, request, value, index, bytes, size, timeout, ret);
  return ret;
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

unsigned short UsbDevice::GetVendorId() const {
  ISCRCPY_ASSERT(dev_, "device can not be null!");
  return dev_->descriptor.idVendor;
}

unsigned short UsbDevice::GetProductId() const {
  ISCRCPY_ASSERT(dev_, "device can not be null!");
  return dev_->descriptor.idProduct;
}

int UsbDevice::BulkRead(unsigned char endpoint_address, const char* buf, size_t size, int timeout) {
  ISCRCPY_ASSERT(dev_h_, "device handle can not be null!");
  int ret = usb_bulk_read(dev_h_, endpoint_address, const_cast<char*>(buf), size, timeout);
  ISCRCPY_LOG_V("usb_bulk_read, dev=%p, ep=0x%x, bytes=%p, size=%zu, timeout=%d, ret=%d\n", dev_h_,
                endpoint_address, buf, size, timeout, ret);
#if ISCRCPY_DEBUG
  if (ret > 0) {
    hexdump(const_cast<char*>(buf), ret, 0);
  }
#endif
  return ret;
}

int UsbDevice::BulkWrite(unsigned char endpoint_address, const char* buf, size_t size,
                         int timeout) {
  ISCRCPY_ASSERT(dev_h_, "device handle can not be null!");
  int ret = usb_bulk_write(dev_h_, endpoint_address, const_cast<char*>(buf), size, timeout);
  ISCRCPY_LOG_V("usb_bulk_write, dev=%p, ep=0x%x, bytes=%p, size=%zu, timeout=%d, ret=%d\n", dev_h_,
                endpoint_address, buf, size, timeout, ret);
#if ISCRCPY_DEBUG
  hexdump(const_cast<char*>(buf), size, 0);
#endif
  return ret;
}

std::string UsbDevice::LastError() const { return usb_strerror(); }

}  // namespace scrcpy_ios

#endif  // LIBUSB_WIN32