#include "scrcpy_ios/usbdevice.h"

#include <sstream>
#include <codecvt> 

#include "scrcpy_ios/macro_def.h"
#include "scrcpy_ios/utils.h"

namespace scrcpy_ios {

#ifdef LIBUSB

constexpr char ENDPOINT_NUM_MASK = LIBUSB_ENDPOINT_ADDRESS_MASK;
constexpr char ENDPOINT_DIRECTION_MASK = LIBUSB_ENDPOINT_DIR_MASK;

UsbDevice::UsbDevice(libusb_device* dev) : dev_(dev), usbmuxio_handle_(0){
	libusb_ref_device(dev_);

	int ret = libusb_get_device_descriptor(dev, &desc_);
	if (ret < 0) {
		fprintf(stderr, "failed to get device descriptor");
	}
}

UsbDevice::~UsbDevice() {
	libusb_unref_device(dev_);
}

bool UsbDevice::FindInterface(unsigned char interface_class, unsigned char interface_sub_class,
                              UsbInterface& result) const {
    for (int i = 0; i < desc_.bNumConfigurations; ++i) {
        struct libusb_config_descriptor* config_desc;
        int ret = libusb_get_config_descriptor(dev_, i, &config_desc);
        if (LIBUSB_SUCCESS != ret) {
            printf("Couldn't retrieve descriptors\n");
            continue;
        }

        for (int l = 0; l < config_desc->bNumInterfaces; ++l) {
            const struct libusb_interface* usb_interface = &config_desc->interface[l];
            for (int x = 0; x < usb_interface->num_altsetting; ++x) {
                const struct libusb_interface_descriptor* altsetting = &usb_interface->altsetting[x];
                if (altsetting->bInterfaceClass == interface_class &&
                    altsetting->bInterfaceSubClass == interface_sub_class) {
                    result.config_desc_num = config_desc->bConfigurationValue;
                    result.interface_num = altsetting->bInterfaceNumber;
                    result.endpoints_size = altsetting->bNumEndpoints;
                    for (int y = 0; y < altsetting->bNumEndpoints; ++y) {
                        const struct libusb_endpoint_descriptor* endpoint = &altsetting->endpoint[y];
                        ISCRCPY_LOG_V("Found endpoint bEndpointAddress=%d, wMaxPacketSize=%d\n",
                            endpoint->bEndpointAddress, endpoint->wMaxPacketSize);
                        result.endpoints[y].address = endpoint->bEndpointAddress;
                        result.endpoints[y].number = endpoint->bEndpointAddress & ENDPOINT_NUM_MASK;
                        result.endpoints[y].direction =
                            (endpoint->bEndpointAddress & ENDPOINT_DIRECTION_MASK) == LIBUSB_ENDPOINT_IN
                            ? UsbEndpointDirection::kIn
                            : UsbEndpointDirection::kOut;
                    }
                    return true;
                }
            }
        }
    }
}

int UsbDevice::SetConfiguration(unsigned char configuration_num) {
  return 0; // TODO
}

int UsbDevice::ClaimInterface(unsigned char interface_num) {
  return 0; // TODO
}

int UsbDevice::ReleaseInterface(unsigned char interface_num) {
  return 0; // TODO
}

int UsbDevice::ClearHalt(unsigned char endpoint_address) {
  return 0; // TODO
}

bool UsbDevice::Open() {
    std::wstring device_path = GetDevicePath();
  return 0; // TODO
}

bool UsbDevice::IsOpened() const {
    return false; // TODO
}

bool UsbDevice::Reopen() {
    return false; // TODO
}

bool UsbDevice::Close() {
    return false; // TODO
}

int UsbDevice::Control(int requesttype, int request, int value, int index, char* bytes, int size,
                       int timeout) const {
    return 0; // TODO
}

std::string UsbDevice::GetName() const {
  std::stringstream ss;
  ss << "VID_" << std::hex << desc_.idVendor << "&PID_" << desc_.idProduct;
  return ss.str();
}

std::wstring UsbDevice::GetDevicePath() const {
    uint8_t path[MAX_PATH];
    int ret = libusb_get_port_path(NULL, dev_, path, sizeof(path));
    if (ret == LIBUSB_SUCCESS) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        //return conv.from_bytes(path);
        printf("devicePath: %s\n", path);
    }
    return L"";
}


std::string UsbDevice::GetSerialNumber() const {
  return std::string(); // TODO
}

int UsbDevice::BulkRead(unsigned char endpoint_address, const char* buf, size_t size, int timeout) {
  return 0; // TODO
}

int UsbDevice::BulkWrite(unsigned char endpoint_address, const char* buf, size_t size,
                         int timeout) {
  return 0; // TODO
}

std::string UsbDevice::LastError() const {
    return ""; // TODO
}

unsigned short UsbDevice::GetVendorId() const {
    ISCRCPY_ASSERT(dev_, "device can not be null!");
    return desc_.idVendor;
}

unsigned short UsbDevice::GetProductId() const {
    ISCRCPY_ASSERT(dev_, "device can not be null!");
    return desc_.idProduct;
}


// static
std::vector<std::unique_ptr<UsbDevice>> FindUsbDevices(UsbDeviceFilter filter) {
    std::vector<std::unique_ptr<UsbDevice>> devices;

    // TODO:
    int r, i;
    r = libusb_init_context(/*ctx=*/NULL, /*options=*/NULL, /*num_options=*/0);
    //r = libusb_init(&ctx);
    if (r < 0) {
        return devices;
    }
    //libusb_set_option(NULL, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_DEBUG);
    // ------------------------------------------------------------------------

    libusb_device** devs;
    ssize_t cnt;
	cnt = libusb_get_device_list(NULL, &devs);
	if (cnt < 0) {
		libusb_exit(NULL);
		return devices;
	}

    for (i = 0; devs[i]; i++) {
        std::unique_ptr<UsbDevice> device = std::make_unique<UsbDevice>(devs[i]);
        if (filter(device.get())) {
            devices.emplace_back(std::move(device));
        }
    }

    libusb_free_device_list(devs, 1);
	return devices;
}

#endif  // LIBUSB

}  // namespace scrcpy_ios
