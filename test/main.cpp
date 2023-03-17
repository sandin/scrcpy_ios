#include <iostream>

#include "scrcpy_ios/macro_def.h"
#include "scrcpy_ios/screenrecorder.h"
#include "scrcpy_ios/usbdevice.h"

using namespace scrcpy_ios;

// Device vendor and product id.
#define IPHONE_VID 0x05AC
#define IPHONE_PID 0x12A8

// UsbMuxSubclass is the subclass used for USBMux USB configuration.
#define USBMUX_SUBCLASS 0xFE
// QuicktimeSubclass is the subclass used for the Quicktime USB configuration.
#define QUICKTIME_SUBCLASS 0x2A

int main() {
  ISCRCPY_LOG_I("scrcpy_ios\n");

  UsbDevice* device = nullptr;
  UsbInterface usbmux_interface = {0};
  UsbInterface quicktime_interface = {0};
  int retry = 0;
  while (retry < 10) {
    device = FindUsbDevice([](unsigned short vid, unsigned short pid) {
      return vid == IPHONE_VID && pid == IPHONE_PID;
    });
    if (device) {
      if (device->Open()) {
        bool found_usbmux =
            device->FindInterface(USB_CLASS_VENDOR_SPEC, USBMUX_SUBCLASS, usbmux_interface);
        bool found_quicktime =
            device->FindInterface(USB_CLASS_VENDOR_SPEC, QUICKTIME_SUBCLASS, quicktime_interface);
        ISCRCPY_LOG_D("Found a iOS device %s, usbmux=%d, quicktime=%d\n", device->GetName().c_str(),
                      found_usbmux, found_quicktime);
        if (found_usbmux && found_quicktime) {
          break;  // found
        }

        ISCRCPY_LOG_D("Open the iOS device %s, SerialNumber=%s\n", device->GetName().c_str(),
                      device->GetSerialNumber().c_str());
        bool enabled = ScreenRecorder::EnableQuicktimeConfigDesc(device);
        ISCRCPY_LOG_D("EnableQuicktimeConfigDesc, success=%d\n", enabled);
        device->Close();  // and reopen
      }
      FreeUsbDevice(device);
      device = nullptr;
    }
    Sleep(30 * 1000);  // NOTE: DO NOT DELETE THIS LINE
    retry++;
  }

  if (!device) {
    ISCRCPY_LOG_E("Can not find any iOS device\n");
    goto EXIT;
  }

  int ret = device->SetConfiguration(quicktime_interface.config_desc_num);
  ISCRCPY_LOG_D("Set Configuration %d, ret=%d\n", quicktime_interface.config_desc_num, ret);
  if (ret != UsbDevice::Ok) {
    ISCRCPY_LOG_E("Can not set configuration to quicktime interface\n");
    goto EXIT;
  }

  ret = device->ClaimInterface(quicktime_interface.interface_num);
  ISCRCPY_LOG_D("Claim Interface %d, ret=%d\n", quicktime_interface.config_desc_num, ret);
  if (ret != UsbDevice::Ok) {
    ISCRCPY_LOG_E("Can not claim quicktime interface\n");
    goto EXIT;
  }

  const UsbEndpoint* in_endpoint = nullptr;
  const UsbEndpoint* out_endpoint = nullptr;
  for (int i = 0; i < quicktime_interface.endpoints_size; ++i) {
    if (quicktime_interface.endpoints[i].direction == UsbEndpointDirection::kIn) {
      in_endpoint = &quicktime_interface.endpoints[i];
    } else if (quicktime_interface.endpoints[i].direction == UsbEndpointDirection::kOut) {
      out_endpoint = &quicktime_interface.endpoints[i];
    }
  }
  if (in_endpoint == nullptr) {
    ISCRCPY_LOG_E("Can not found in endpoint of quicktime interface\n");
    goto EXIT;
  }
  if (out_endpoint == nullptr) {
    ISCRCPY_LOG_E("Can not found out endpoint of quicktime interface\n");
    goto EXIT;
  }

  bool success = ScreenRecorder::ClearFeature(device, in_endpoint->address);
  if (!success) {
    ISCRCPY_LOG_E("Can not clear feature for in endpoint\n");
    goto EXIT;
  }
  success = ScreenRecorder::ClearFeature(device, out_endpoint->address);
  if (!success) {
    ISCRCPY_LOG_E("Can not clear feature for out endpoint\n");
    goto EXIT;
  }

  device->ClearHalt(in_endpoint->address);
  device->ClearHalt(out_endpoint->address);

  ret = device->ReleaseInterface(quicktime_interface.interface_num);
  if (ret != UsbDevice::Ok) {
    ISCRCPY_LOG_E("Can not release interface\n");
    goto EXIT;
  }

  // ScreenRecorder recorder(device);

EXIT:
  device->Close();
  FreeUsbDevice(device);
  return 0;
}