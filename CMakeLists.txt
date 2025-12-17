# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(pico_spectrum C CXX ASM)
set(PICO_SDK_FETCH_FROM_GIT on)
set(PICO_CMSIS_DSP OFF)

pico_sdk_init()

# Option to enable display test mode
option(ENABLE_DISPLAY_TEST "Enable display test mode" OFF)

add_executable(pico_spectrum
    src/main.c
    src/adc_mcp3202.c
    src/dsp.c
    src/dsp_time.c
    src/display.c
    src/audio_out_pwm.c
    src/debug_usb.c
    src/ht16k33.c
)

target_link_libraries(pico_spectrum
    pico_stdlib
    pico_multicore
    hardware_spi
    hardware_i2c
    hardware_dma
    hardware_pwm
    hardware_timer
)


# --- Include directories ---
target_include_directories(pico_spectrum PRIVATE src)

# --- Compiler warnings / options ---
target_compile_options(pico_spectrum PRIVATE
    -Wall
    -Wextra
    -Werror=implicit-function-declaration
)

pico_enable_stdio_usb(pico_spectrum 1)
pico_enable_stdio_uart(pico_spectrum 0)

pico_add_extra_outputs(pico_spectrum)

# --- Preprocessor macro for display test ---
if(ENABLE_DISPLAY_TEST)
    message(STATUS "Building with DISPLAY_TEST enabled")
    target_compile_definitions(pico_spectrum PRIVATE DISPLAY_TEST)
endif()


