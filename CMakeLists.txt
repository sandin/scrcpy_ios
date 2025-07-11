cmake_minimum_required(VERSION 3.0.0)
project(scrcpy_ios VERSION 0.1.0)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)

option(USE_LIBUSB_WIN32 Off)

IF (${USE_LIBUSB_WIN32})
    # libusb-win32
    include_directories("./libs/libusb-win32/include")
    link_directories("./libs/libusb-win32/lib/msvc_x64")
    add_definitions(-DLIBUSB_WIN32)
    set(LIBUSB_LIB libusb)
    set(LIBUSB_SOURCES
        src/usbdevice_libusb_win32.cpp
    )
ELSE()
    # libusb
    include_directories("./libs/libusb/include")
    link_directories("./libs/libusb/lib")
    add_definitions(-DLIBUSB)
    set(LIBUSB_LIB libusb-1.0)
    set(LIBUSB_SOURCES
        src/usbdevice_libusb.cpp
    )
ENDIF()

# library: scrcpy_ios
include_directories("./include")
set(HEADER_FILES
    include/scrcpy_ios/screenrecorder.h
    include/scrcpy_ios/usbdevice.h
    include/scrcpy_ios/quicktime_protocol.h
    include/scrcpy_ios/devicemanager.h
    include/scrcpy_ios/usbmuxio.h
)
set(SOURCE_FILES
    src/devicemanager.cpp
    src/quicktime_protocol.cpp
    src/screenrecorder.cpp
    src/usbmuxio.cpp
    ${LIBUSB_SOURCES}
)


add_library(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${LIBUSB_LIB})

# exe: scrcpy_ios_test
add_executable(${PROJECT_NAME}_test
    test/main.cpp
)
target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME})