#include <assert.h>

#include <iostream>

#include "scrcpy_ios/screenrecorder.h"
#include "scrcpy_ios/usbdevice.h"

using namespace scrcpy_ios;

// Device vendor and product id.
#define IPHONE_VID 0x05AC
#define IPHONE_PID 0x12A8

// UsbMuxSubclass is the subclass used for USBMux USB configuration.
#define USBMUX_SUBCLASS 0xFE
// QuicktimeSubclass is the subclass used for the Quicktime USB configuration.
#define QUICKTIME_SUBCLASS 0x2

int main() {
  std::cout << "scrcpy_ios" << std::endl;


  UsbDevice* device = nullptr;
  int retry = 0;
  while (retry < 10) {
      device = FindUsbDevice([](unsigned short vid, unsigned short pid) {
          return vid == IPHONE_VID && pid == IPHONE_PID;
      });
      if (device) {
          bool usbmux_enabled = device->FindConfigDesc(USB_CLASS_VENDOR_SPEC, USBMUX_SUBCLASS);
          bool quicktime_enabled = device->FindConfigDesc(USB_CLASS_VENDOR_SPEC, QUICKTIME_SUBCLASS);
          if (usbmux_enabled && quicktime_enabled) {
              break; // found
          }

          if (device->Open()) {
              ScreenRecorder::EnableQuicktimeConfigDesc(device);
              device->Close(); // and reopen
          }
          FreeUsbDevice(device);
          device = nullptr;
      }
      retry++;
  }

  if (!device) {
      std::cout << "Error: can not find any iOS device" << std::endl;
      return -1;
  }

  ScreenRecorder recorder(device);


  return 0;
}