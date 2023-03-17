cmake_minimum_required(VERSION 3.0.0)
project(scrcpy_ios VERSION 0.1.0)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)

IF (WIN32)
    # libusb-win32
    include_directories("./libs/libusb-win32/include")
    link_directories("./libs/libusb-win32/lib/msvc_x64")
    add_definitions(-DLIBUSB_WIN32)
ELSE()
    # libusb
    # TODO: 
    add_definitions(-DLIBUSB_WIN32)
    add_definitions(-DLIBUSB)
ENDIF()

# library: scrcpy_ios
include_directories("./include")
set(HEADER_FILES
    include/scrcpy_ios/screenrecorder.h
    include/scrcpy_ios/usbdevice.h
    include/scrcpy_ios/devicemanager.h
)
set(SOURCE_FILES
    src/usbdevice_win.cpp
    src/devicemanager.cpp
    src/screenrecorder.cpp
)

add_library(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} libusb)

# exe: scrcpy_ios_test
add_executable(${PROJECT_NAME}_test
    test/main.cpp
)
target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME})