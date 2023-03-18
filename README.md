# scrcpy_ios

Screencast for iPhone.


## Requirement

* libusb-win32 (for windows)
    * Download [libusb-win32-devel-filter](libusb-win32-devel-filter-1.2.7.3.exe) and install this filter driver for your iOS device.
    * Download [libusb-win32-bin](libusb-win32-bin-1.2.7.3.zip
) and unzip it to `libs/libusb-win32` directory.

## Build

Windows msvc:
```
$ mkdir build && cd build
$ cmake -G "Visual Studio 16 2019" ..
```

then open `build/scrcpy_ios.sln` .