cmake_minimum_required(VERSION 3.13)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(led_blink C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(led_blink
    main.c
)

target_link_libraries(led_blink pico_stdlib)
pico_enable_stdio_usb(led_blink 1) # USB CDC 활성화
pico_add_extra_outputs(led_blink) 