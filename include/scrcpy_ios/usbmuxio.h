#ifndef SCRCPY_IOS_USBMUXIO_H
#define SCRCPY_IOS_USBMUXIO_H

namespace scrcpy_ios {

using UsbmuxioHandle = HANDLE;


UsbmuxioHandle Usbmuxio_OpenDevice(std::wstring path);

}  // namespace scrcpy_ios

#endif  // SCRCPY_IOS_USBMUXIO_