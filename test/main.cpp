#include <memory>  // unique_ptr

#include "scrcpy_ios/devicemanager.h"
#include "scrcpy_ios/macro_def.h"
#include "scrcpy_ios/screenrecorder.h"
#include "scrcpy_ios/usbdevice.h"

using namespace scrcpy_ios;

int main() {
  ISCRCPY_LOG_I("scrcpy_ios\n");

  std::string usb_serial = "";  // TODO: arg

  // Find the device
  std::unique_ptr<UsbDevice> device;
  if (!usb_serial.empty()) {
    device = DeviceManager::FindIosDevice(usb_serial);
  } else {
    std::vector<std::unique_ptr<UsbDevice>> devices = DeviceManager::FindAllIosDevices();
    if (devices.size() > 0) {
      device = std::move(devices[0]);
    }
  }
  if (!device) {
    ISCRCPY_LOG_E("Can not find the usb device, usb_serial=%s\n", usb_serial);
    return -1;
  }

  // Start screen recording
  ScreenRecorder recorder(std::move(device));
  int frame_count = 0;
  recorder.SetVideoFrameCallback([&](const char* buf, size_t size) {
    frame_count++;
    return frame_count < 10;
  });
  recorder.SetAudioSampleCallback([](const char* buf, size_t size) { return true; });
  ScreenRecorder::Result ret = recorder.StartRecording();
  if (ret != ScreenRecorder::Result::kOk) {
    ISCRCPY_LOG_E("Can not start recording, ret=%d\n", ret);
    return -1;
  }

  recorder.StopRecording();
  return 0;
}