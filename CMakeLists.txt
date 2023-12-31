cmake_minimum_required(VERSION 3.19)

# need to fetch the SDK

# download CPM.cmake
file(
  DOWNLOAD
  https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.38.3/CPM.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
  EXPECTED_HASH SHA256=cc155ce02e7945e7b8967ddfaff0b050e958a723ef7aad3766d368940cb15494
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

# boilerplate from the SDK's github README
set(PICO_SDK_FETCH_FROM_GIT on)

include(cmake/pico_sdk_import.cmake)

project(serial-neopixel)

pico_sdk_init()

add_executable(${PROJECT_NAME} 
    app/firmware.cpp
    app/led_driver.cpp
    app/pico_panic.cpp
    app/pico_chrono.cpp
    commands/help.cpp
    commands/set.cpp
    commands/pattern.cpp
    commands/clock.cpp
)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_link_libraries(${PROJECT_NAME} PRIVATE 
    pico_printf
    pico_stdlib 
    pio_ws2812 
    pico_time
    hardware_pwm
    )
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})
pico_add_extra_outputs(${PROJECT_NAME})

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

add_subdirectory(ws2812)
