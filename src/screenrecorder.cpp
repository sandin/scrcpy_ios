#include "scrcpy_ios/screenrecorder.h"

#include <cstdint>

#include "scrcpy_ios/devicemanager.h"
#include "scrcpy_ios/macro_def.h"

using namespace scrcpy_ios;
using namespace scrcpy_ios::quicktime_protocol;

constexpr int kWriteTimeout = 5000;
constexpr int kReadTimeout = 50 * 1000; // TODO

// static
bool ScreenRecorder::EnableQuicktimeConfigDesc(UsbDevice* dev) {
  ISCRCPY_ASSERT(dev, "device can not be null!");
  return dev->Control(0x40, 0x52, 0x01 /* value */, 0x02 /* index */, NULL, 0, 1000) == UsbDevice::Ok;
}

// static
bool ScreenRecorder::DisableQuicktimeConfigDesc(UsbDevice* dev) {
  ISCRCPY_ASSERT(dev, "device can not be null!");
  return dev->Control(0x40, 0x52, 0x00, 0x00, NULL, 0, 1000) == UsbDevice::Ok;
}

// static
bool ScreenRecorder::EnableCDCNCMConfigDesc(UsbDevice* dev) {
    ISCRCPY_ASSERT(dev, "device can not be null!");
    return dev->Control(0x40, 0x52, 0x01 /* value */, 0x03 /* index */, NULL, 0, 1000) == UsbDevice::Ok;
}

// static
bool ScreenRecorder::ClearFeature(UsbDevice* dev, unsigned char endpoint_address) {
  ISCRCPY_ASSERT(dev, "device can not be null!");
  return dev->Control(0x02, 0x01, 0x00, endpoint_address, NULL, 0, 1000) == UsbDevice::Ok;
}

// static
bool ScreenRecorder::FindInterface(UsbDevice* dev, unsigned char interface_sub_class,
                                   UsbInterface& result) {
  return dev->FindInterface(kVendorSpecInterfaceclass, interface_sub_class, result);
}

ScreenRecorder::Result ScreenRecorder::Prepare() {
  ISCRCPY_ASSERT(dev_);
  if (!dev_->Open()) {
    return Result::kErrCanNotOpenDevice;
  }

  bool found_usbmux, found_quicktime = false;
  int retry = 0;
  while (retry < 5) {
    found_usbmux = FindInterface(dev_.get(), kUsbmuxInterfaceClass, usbmux_interface_);
    found_quicktime =
        FindInterface(dev_.get(), kQuicktimeInterfaceClass, quicktime_interface_);
    ISCRCPY_LOG_D("Found a iOS device %s, usbmux=%d, quicktime=%d\n", dev_->GetName().c_str(),
                  found_usbmux, found_quicktime);
    if (found_usbmux && found_quicktime) {
      break;  // the device has quicktime turned on
    }

    if (!found_quicktime) {
      bool success = ScreenRecorder::EnableQuicktimeConfigDesc(dev_.get());
      ISCRCPY_LOG_D("Enable Quicktime Config Desc, result=%d\n", success);
      if (!success) {
        dev_->Close();
        return Result::kErrCanNotEnableQuicktime;
      }

      ISCRCPY_LOG_D("Start reopening the device...\n");
      success = dev_->Reopen();
      ISCRCPY_LOG_D("reopen result=%d\n", success);
      if (!success) {
        return Result::kErrCanNotOpenDevice;
      }
    }
    retry++;
  }
  if (!found_quicktime) {
      dev_->Close();
      return Result::kErrCanNotEnableQuicktime;
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
  ISCRCPY_LOG_D("Set configuration to quicktime interface(config_desc_num=%d), ret=%d\n",
                quicktime_interface_.config_desc_num, ret);
  if (ret != UsbDevice::Ok) {
    return Result::kErrCanNotSetConfig;
  }

  ret = dev_->ClaimInterface(quicktime_interface_.interface_num);
  ISCRCPY_LOG_D("Claim quicktime interface(num=%d), ret=%d\n", quicktime_interface_.interface_num,
                ret);
  if (ret != UsbDevice::Ok) {
    return Result::kErrCanNotClaimInterface;
  }

  for (int i = 0; i < quicktime_interface_.endpoints_size; ++i) {
    if (quicktime_interface_.endpoints[i].direction == UsbEndpointDirection::kIn) {
      in_endpoint_ = 0x80 | quicktime_interface_.endpoints[i].number;
    } else if (quicktime_interface_.endpoints[i].direction == UsbEndpointDirection::kOut) {
      out_endpoint_ = quicktime_interface_.endpoints[i].number;
    }
  }
  if (in_endpoint_ == 0) {
    return Result::kErrCanNotFoundEndpoint;
  }
  if (out_endpoint_ == 0) {
    return Result::kErrCanNotFoundEndpoint;
  }

  bool success = ClearFeature(dev_.get(), in_endpoint_);
  ISCRCPY_LOG_D("Clear feature of in endpoint(address=%d), result=%d\n", in_endpoint_, success);
  if (!success) {
    return Result::kErrCanNotClearFeature;
  }
  success = ScreenRecorder::ClearFeature(dev_.get(), out_endpoint_);
  ISCRCPY_LOG_D("Clear feature of out endpoint(address=%d), result=%d\n", out_endpoint_, success);
  if (!success) {
    return Result::kErrCanNotClearFeature;
  }

  ret = dev_->ClearHalt(in_endpoint_);
  ISCRCPY_LOG_D("Clear halt of in endpoint(address=0x%x), ret=%d\n", in_endpoint_, success);
  ret = dev_->ClearHalt(out_endpoint_);
  ISCRCPY_LOG_D("Clear halt of out endpoint(address=0x%x), ret=%d\n", out_endpoint_, success);

  return Result::kOk;
}

ScreenRecorder::Result ScreenRecorder::StartRecording() {
  Result result = Prepare();
  if (result != Result::kOk) {
    return result;
  }

  SendPacket(PingPacket());

  ISCRCPY_LOG_D("Start a loop to receive message from endpoint.\n");
  recording_.store(true, std::memory_order_release);
  char* payload_buffer = static_cast<char*>(malloc(0x500000));
  while (recording_) {
      int read = dev_->BulkRead(in_endpoint_, payload_buffer, 0x500000, -1);
      if (read < 0) {
          ISCRCPY_LOG_E("Can not read the payload of packet, err=%s.\n", dev_->LastError().c_str());
          break;
      }

    /*
    // read header
    uint32_t len = 0;
    int read = dev_->BulkRead(in_endpoint_, reinterpret_cast<const char*>(&len), sizeof(uint32_t), kReadTimeout);
    if (read < 0) {
        ISCRCPY_LOG_E("Can not read the header of packet, err=%s.\n", dev_->LastError().c_str());
        break;
    }
    ISCRCPY_LOG_D("receive packet header, expect=%zu, actual=%d\n", sizeof(uint32_t), read);

    // read payload
    uint32_t payload_size = len - sizeof(uint32_t);
    payload_buffer = static_cast<char*>(malloc(payload_size)); // TODO: avoid alloc memory in the loop
    read = dev_->BulkRead(in_endpoint_, payload_buffer, payload_size, kReadTimeout);
    if (read < 0) {
        ISCRCPY_LOG_E("Can not read the payload of packet, err=%s.\n", dev_->LastError().c_str());
        break;
    }

    ISCRCPY_LOG_D("receive packet payload, expect=%d, actual=%d\n", payload_size, read);
    result = HandlePacket(payload_buffer, payload_size);
    if (result != Result::kOk) {
        ISCRCPY_LOG_E("Can not handle packet.\n");
        break;
    }
    */

  }
  free(payload_buffer);
  ISCRCPY_LOG_D("End the loop to receive message from endpoint.\n");

  return Result::kOk;  
}

ScreenRecorder::Result ScreenRecorder::HandlePacket(const char* payload_buffer, size_t payload_size) {
  if (payload_size > 4) {
      uint32_t magic = *(uint32_t*)payload_buffer;
      switch (magic) {
      case PingPacket::kMagic: 
          ISCRCPY_LOG_D("Got a Ping packet\n");
          return SendPacket(PingPacket());
        // TODO: other cases
        default:
            break; // TODO
      }
  }
  return Result::kErrUnexpectedPacket;
}

ScreenRecorder::Result ScreenRecorder::SendPacket(const quicktime_protocol::Packet& packet) {
    ScreenRecorder::Result result;
    size_t size = 0;
    char* buf;
    packet.Serialize(&buf, &size);

	int ret = dev_->BulkWrite(out_endpoint_, buf, size, kWriteTimeout);
    result = ret == 0 ? Result::kOk : Result::kErrIOPacket;
	free(buf);
    return result;
}

ScreenRecorder::Result ScreenRecorder::StopRecording() {
  recording_.store(false, std::memory_order_release);

  int ret = dev_->ReleaseInterface(quicktime_interface_.interface_num);
  if (ret != UsbDevice::Ok) {
    ISCRCPY_LOG_E("Can not release interface, ret=%d\n", ret);
  }

  // TODO

  return Result::kErrUnknown;  // TODO
}
