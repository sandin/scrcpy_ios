#include "scrcpy_ios/screenrecorder.h"

#include "scrcpy_ios/macro_def.h"

using namespace scrcpy_ios;

constexpr int kVendorSpecInterfaceclass = 0xFF;
constexpr int kUsbmuxInterfaceSubclass = 0xFE;
constexpr int kQuicktimeInterfaceSubclass = 0x2A;

// static
bool ScreenRecorder::EnableQuicktimeConfigDesc(UsbDevice* dev) {
  ISCRCPY_ASSERT(dev, "device can not be null!");
  return dev->Control(0x40, 0x52, 0x00, 0x02, NULL, 0, 1000) == UsbDevice::Ok;
}

// static
bool ScreenRecorder::DisableQuicktimeConfigDesc(UsbDevice* dev) {
  ISCRCPY_ASSERT(dev, "device can not be null!");
  return dev->Control(0x40, 0x52, 0x00, 0x00, NULL, 0, 1000) == UsbDevice::Ok;
}

// static
bool ScreenRecorder::ClearFeature(UsbDevice* dev, unsigned char endpoint_address) {
  ISCRCPY_ASSERT(dev, "device can not be null!");
  return dev->Control(0x02, 0x01, 0x00, endpoint_address, NULL, 0, 1000) == UsbDevice::Ok;
}

int ScreenRecorder::Prepare() {
  if (device->Open()) {
    return -1;  // Can not open the device
  }

  UsbInterface usbmux_interface = {0};
  UsbInterface quicktime_interface = {0};
  bool found_usbmux =
      device->FindInterface(kVendorSpecInterfaceclass, kUsbmuxInterfaceSubclass, usbmux_interface);
  bool found_quicktime = device->FindInterface(kVendorSpecInterfaceclass,
                                               kQuicktimeInterfaceSubclass, quicktime_interface);
  ISCRCPY_LOG_D("Found a iOS device %s, usbmux=%d, quicktime=%d\n", device->GetName().c_str(),
                found_usbmux, found_quicktime);
  if (!found_usbmux) {
    return -2;  // It's not a valid iPhone device
  }

  if (!found_quicktime) {
    ScreenRecorder::EnableQuicktimeConfigDesc(device);
    if (device->Reopen()) {
      return -1;  // Can not open the device
    }
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

EXIT:
  device->Close();
  return 0;
}

int ScreenRecorder::StartRecording() {
  int ret = Prepare();
  if (ret != 0) {
    return ret;
  }

  return -1;  // TODO
}

int ScreenRecorder::StopRecording() {
  return -1;  // TODO
}
