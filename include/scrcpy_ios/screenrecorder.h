#ifndef SCRCPY_IOS_SCREEN_RECORDER_H
#define SCRCPY_IOS_SCREEN_RECORDER_H

#include <memory>

#include "scrcpy_ios/usbdevice.h"

namespace scrcpy_ios {

class ScreenRecorder {
 public:
  using VideoFrameCallback = std::function<bool(const char*, size_t)>;
  using AudioSampleCallback = std::function<bool(const char*, size_t)>;

  enum class Result {
    kOk = 0,
    kErrCanNotOpenDevice = -1,
    kErrNotValidDevice = -2,
    kErrCanNotEnableQuicktime = -3,
    kErrCanNotSetConfig = -4,
    kErrCanNotClaimInterface = -5,
    kErrCanNotFoundEndpoint = -6,
    kErrCanNotClearFeature = -7,
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
  static bool ClearFeature(UsbDevice* dev, unsigned char endpoint_address);

  Result Prepare();
  Result SetConfigAndClaimInterface();

  std::unique_ptr<UsbDevice> dev_;
  VideoFrameCallback video_frame_callback_;
  AudioSampleCallback audio_sample_callback_;

  UsbInterface usbmux_interface_ = {0};
  UsbInterface quicktime_interface_ = {0};
  UsbEndpoint* in_endpoint_ = nullptr;
  UsbEndpoint* out_endpoint_ = nullptr;
};

}  // namespace scrcpy_ios

#endif  // SCRCPY_IOS_SCREEN_RECORDER_H