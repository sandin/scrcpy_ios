#ifndef SCRCPY_IOS_SCREEN_RECORDER_H
#define SCRCPY_IOS_SCREEN_RECORDER_H

#include <memory>

#include "scrcpy_ios/usbdevice.h"

namespace scrcpy_ios {

class ScreenRecorder {
 public:
  using VideoFrameCallback = std::function<bool(const char*, size_t)>;
  using AudioSampleCallback = std::function<bool(const char*, size_t)>;

  ScreenRecorder(std::unique_ptr<UsbDevice> dev) : dev_(std::move(dev)) {}

  ~ScreenRecorder() {}

  void SetVideoFrameCallback(VideoFrameCallback callback) { video_frame_callback_ = callback; }
  void SetAudioSampleCallback(AudioSampleCallback callback) { audio_sample_callback_ = callback; }

  int StartRecording();
  int StopRecording();

 private:
  static bool EnableQuicktimeConfigDesc(UsbDevice* dev);
  static bool DisableQuicktimeConfigDesc(UsbDevice* dev);
  static bool ClearFeature(UsbDevice* dev, unsigned char endpoint_address);

  int Prepare();

  std::unique_ptr<UsbDevice> dev_;
  VideoFrameCallback video_frame_callback_;
  AudioSampleCallback audio_sample_callback_;
};

}  // namespace scrcpy_ios

#endif  // SCRCPY_IOS_SCREEN_RECORDER_H