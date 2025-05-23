cmake_minimum_required(VERSION 3.13)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(led_task C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(FREERTOS_KERNEL_PATH $ENV{FREERTOS_KERNEL_PATH})
set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
# set(PICO_PLATFORM rp2040)

# Pico SDK의 CMake 설정 초기화
pico_sdk_init() 

# Src파일들을 묶어 FreeRTOS-Kernel 타겟에 저장
add_library(FreeRTOS-Kernel STATIC

    ${FREERTOS_KERNEL_PATH}/tasks.c
    ${FREERTOS_KERNEL_PATH}/queue.c
    ${FREERTOS_KERNEL_PATH}/list.c
    ${FREERTOS_KERNEL_PATH}/timers.c
    ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/port.c
    ${FREERTOS_KERNEL_PATH}/portable/MemMang/heap_4.c

)

target_compile_definitions(FreeRTOS-Kernel PUBLIC
    PICO_RP2040=1
)

# FreeRTOS-Kernel 타겟이 컴파일 될 때 사용할 헤더의 경로 지정
target_include_directories(FreeRTOS-Kernel PUBLIC

    ${FREERTOS_KERNEL_PATH}/include
    ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/include

    ${PICO_SDK_PATH}/src/common/pico_base_headers/include

    ${PICO_SDK_PATH}/src/boards/include

    ${PICO_SDK_PATH}/src/rp2040/hardware_structs/include
    ${PICO_SDK_PATH}/src/rp2040/hardware_regs/include
    ${PICO_SDK_PATH}/src/rp2040/pico_platform/include

    ${PICO_SDK_PATH}/src/rp2_common/pico_stdlib/include

    ${PICO_SDK_PATH}/src/rp2_common/pico_platform_compiler/include
    ${PICO_SDK_PATH}/src/rp2_common/pico_platform_sections/include
    ${PICO_SDK_PATH}/src/rp2_common/pico_platform_panic/include

    ${PICO_SDK_PATH}/src/rp2_common/hardware_sync/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_sync_spin_lock/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_clocks/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_exception/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_base/include
    # ${PICO_SDK_PATH}/src/rp2_common/hardware_irq/include

    ${CMAKE_BINARY_DIR}/generated/pico_base
)

# main.c로 실행 파일을 생성
add_executable(led_task
    main.c
)

# led_task 실행 파일을 만들 때 사용할 lib를 링크
target_link_libraries(led_task
    pico_stdlib
    FreeRTOS-Kernel
    hardware_exception
)

pico_enable_stdio_usb(led_task 1)
pico_enable_stdio_uart(led_task 0)

#uf2파일 생성
pico_add_extra_outputs(led_task)