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

ScreenRecorder::Result ScreenRecorder::Prepare() {
  ISCRCPY_ASSERT(dev_);
  if (dev_->Open()) {
    return Result::kErrCanNotOpenDevice;
  }

  bool found_usbmux =
      dev_->FindInterface(kVendorSpecInterfaceclass, kUsbmuxInterfaceSubclass, usbmux_interface_);
  bool found_quicktime = dev_->FindInterface(kVendorSpecInterfaceclass, kQuicktimeInterfaceSubclass,
                                             quicktime_interface_);
  ISCRCPY_LOG_D("Found a iOS device %s, usbmux=%d, quicktime=%d\n", dev_->GetName().c_str(),
                found_usbmux, found_quicktime);
  if (!found_usbmux) {
    dev_->Close();
    return Result::kErrNotValidDevice;
  }

  if (!found_quicktime) {
    bool success = ScreenRecorder::EnableQuicktimeConfigDesc(dev_.get());
    if (!success) {
      dev_->Close();
      return Result::kErrCanNotEnableQuicktime;
    }

    success = dev_->Reopen();
    if (!success) {
      dev_->Close();
      return Result::kErrCanNotOpenDevice;
    }
  }

  Result result = SetConfigAndClaimInterface();
  if (result != Result::kOk) {
    dev_->Close();
    return result;
  }

  ISCRCPY_ASSERT(dev_->IsOpened(), "The device should have been opened after the prepare.");
  return Result::kOk;
}

ScreenRecorder::Result ScreenRecorder::SetConfigAndClaimInterface() {
  ISCRCPY_ASSERT(dev_);

  int ret = dev_->SetConfiguration(quicktime_interface_.config_desc_num);
  if (ret != UsbDevice::Ok) {
    ISCRCPY_LOG_E("Can not set configuration to quicktime interface, ret=%d\n", ret);
    return Result::kErrCanNotSetConfig;
  }

  ret = dev_->ClaimInterface(quicktime_interface_.interface_num);
  if (ret != UsbDevice::Ok) {
    ISCRCPY_LOG_E("Can not claim quicktime interface, ret=%d\n", ret);
    return Result::kErrCanNotClaimInterface;
  }

  for (int i = 0; i < quicktime_interface_.endpoints_size; ++i) {
    if (quicktime_interface_.endpoints[i].direction == UsbEndpointDirection::kIn) {
      in_endpoint_ = &quicktime_interface_.endpoints[i];
    } else if (quicktime_interface_.endpoints[i].direction == UsbEndpointDirection::kOut) {
      out_endpoint_ = &quicktime_interface_.endpoints[i];
    }
  }
  if (in_endpoint_ == nullptr) {
    ISCRCPY_LOG_E("Can not found in endpoint of quicktime interface\n");
    return Result::kErrCanNotFoundEndpoint;
  }
  if (out_endpoint_ == nullptr) {
    ISCRCPY_LOG_E("Can not found out endpoint of quicktime interface\n");
    return Result::kErrCanNotFoundEndpoint;
  }

  bool success = ClearFeature(dev_.get(), in_endpoint_->address);
  if (!success) {
    ISCRCPY_LOG_E("Can not clear feature for in endpoint\n");
    return Result::kErrCanNotClearFeature;
  }
  success = ScreenRecorder::ClearFeature(dev_.get(), out_endpoint_->address);
  if (!success) {
    ISCRCPY_LOG_E("Can not clear feature for out endpoint\n");
    return Result::kErrCanNotClearFeature;
  }

  dev_->ClearHalt(in_endpoint_->address);
  dev_->ClearHalt(out_endpoint_->address);

  return Result::kOk;
}

ScreenRecorder::Result ScreenRecorder::StartRecording() {
  Result ret = Prepare();
  if (ret != Result::kOk) {
    return ret;
  }

  return Result::kErrUnknown;  // TODO
}

ScreenRecorder::Result ScreenRecorder::StopRecording() {
  int ret = dev_->ReleaseInterface(quicktime_interface_.interface_num);
  if (ret != UsbDevice::Ok) {
    ISCRCPY_LOG_E("Can not release interface\n");
    // TODO
  }

  return Result::kErrUnknown;  // TODO
}
