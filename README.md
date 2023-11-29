# scrcpy_ios

Screencast for iPhone.


## Requirement

* libusb-win32 (for windows)
    * Download [libusb-win32-devel-filter](https://sourceforge.net/projects/libusb-win32/files/libusb-win32-releases/1.2.7.3/libusb-win32-devel-filter-1.2.7.3.exe/download) and install this filter driver for your iOS device(Apple Mobile Device USB Composite Device, Service : WINUSB).
    * Download [libusb-win32-bin](https://sourceforge.net/projects/libusb-win32/files/libusb-win32-releases/1.2.7.3/libusb-win32-bin-1.2.7.3.zip/download
) and unzip it to `libs/libusb-win32` directory.

NOTE: we use libusb-win instead of libusb, because we need to use the `usb_control_msg` api.

## Build

Windows msvc:
```
$ mkdir build && cd build
$ cmake -G "Visual Studio 16 2019" ..
```

then open `build/scrcpy_ios.sln` .
