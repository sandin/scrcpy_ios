#ifndef SCRCPY_IOS_SCREEN_RECORDER_H
#define SCRCPY_IOS_SCREEN_RECORDER_H

#include <memory>
#include <atomic>

#include "scrcpy_ios/usbdevice.h"
#include "scrcpy_ios/quicktime_protocol.h"

namespace scrcpy_ios {

class ScreenRecorder {
 public:
  using VideoFrameCallback = std::function<bool(const char*, size_t)>;
  using AudioSampleCallback = std::function<bool(const char*, size_t)>;
  using UsbEndpointAddress = unsigned char;

  enum class Result {
    kOk = 0,
    kErrCanNotOpenDevice = -1,
    kErrNotValidDevice = -2,
    kErrCanNotEnableQuicktime = -3,
    kErrCanNotSetConfig = -4,
    kErrCanNotClaimInterface = -5,
    kErrCanNotFoundEndpoint = -6,
    kErrCanNotClearFeature = -7,
    kErrUnexpectedPacket = -8,
    kErrIOPacket = -9,
    kErrUnknown = -255
  };

  ScreenRecorder(std::unique_ptr<UsbDevice> dev) : dev_(std::move(dev)) {}

  ~ScreenRecorder() {}

  void SetVideoFrameCallback(VideoFrameCallback callback) { video_frame_callback_ = callback; }
  void SetAudioSampleCallback(AudioSampleCallback callback) { audio_sample_callback_ = callback; }

  Result StartRecording();
  Result StopRecording();

 private:
  static bool EnableQuicktimeConfigDesc(UsbDevice* dev);
  static bool DisableQuicktimeConfigDesc(UsbDevice* dev);

  static bool EnableCDCNCMConfigDesc(UsbDevice* dev);

  static bool ClearFeature(UsbDevice* dev, unsigned char endpoint_address);
  static bool FindInterface(UsbDevice* dev, unsigned char interface_sub_class,
                                  UsbInterface& result);

  Result Prepare();
  Result SetConfigAndClaimInterface();

  Result HandlePacket(const char* payload_buffer, size_t payload_size);
  Result SendPacket(const quicktime_protocol::Packet& packet);

  std::unique_ptr<UsbDevice> dev_;
  VideoFrameCallback video_frame_callback_;
  AudioSampleCallback audio_sample_callback_;

  UsbInterface usbmux_interface_ = {0};
  UsbInterface quicktime_interface_ = {0};
  UsbEndpointAddress in_endpoint_ = 0;
  UsbEndpointAddress out_endpoint_ = 0;

  std::atomic_bool recording_ = ATOMIC_VAR_INIT(false);
};

}  // namespace scrcpy_ios

#endif  // SCRCPY_IOS_SCREEN_RECORDER_H