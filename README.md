# Pico Spectrum Analyzer & Audio Processor

A real-time audio spectrum analyzer and time-domain audio processor built on the Raspberry Pi Pico (RP2040).

This project samples audio from an MCP3202 SPI ADC, processes it in real time, displays a 16-band spectrum on a 16×16 LED matrix (HT16K33), and outputs processed audio via PWM with bypass and dry/wet mix control.

The design prioritizes deterministic timing, low CPU usage, and SDK stability (no CMSIS dependencies).


## Features

🎧 Real-time audio input via MCP3202 (SPI + DMA)
🎛️ Time-domain DSP:
* Gain
* Low-pass filter
* Soft clipping
* Dry/wet mix
* True bypass
📊 256-point fixed-point FFT (no CMSIS)
🟩 16-band logarithmic spectrum display
💡 16×16 LED matrix driven by 4× HT16K33
🔊 PWM audio output (DMA-driven, jitter-free)
🧠 Dual-core RP2040 architecture
🐞 USB serial debugging and live control
⚙️ No external libraries required

## Architecture Overview

```
        MCP3202 ADC
            │
        SPI + DMA
            │
     ┌──────▼────────┐
     │ Time-Domain DSP│───► PWM Audio Out
     └──────┬────────┘
            │
        Fixed-Point FFT
            │
       16 Frequency Bands
            │
        HT16K33 Display
```

## Pico hardware core allocations

| Core   | Task                              |
| ------ | --------------------------------- |
| Core 0 | Display updates, USB debug        |
| Core 1 | ADC sampling, DSP, FFT, PWM audio |

This separation is intended to ensure stable audio timing regardless of display or USB activity.

## LED Matrix

The hardware setup uses 4 8x8 LEDs with a HT16K33 backpack that communicates over i2c.
With 4 banks of LEDs oriented in a 4x4 grid as follows

```
+---------+---------+
| LED 0   | LED 1   |
| (0,0)   | (8,0)   |
| ...     | ...     |
| (0,7)   | (8,7)   |
+---------+---------+
| LED 2   | LED 3   |
| (0,8)   | (8,8)   |
| ...     | ...     |
| (0,15)  | (8,15)  |
+---------+---------+
Top-left module → LED 0
Top-right → LED 1
Bottom-left → LED 2
Bottom-right → LED 3

Each module has its own 8×8 framebuffer to communicate over I2C.
```

## Hardware

🔌 Hardware Requirements
Required
* Raspberry Pi Pico (RP2040)
* MCP3202 (12-bit SPI ADC)
* 4 × HT16k33 LED drivers
* 16×16 LED matrix (or 4× 8×8)
* RC low-pass filter for PWM output
* Audio amplifier (for speaker output)


## Repository Structure

```
pico-spectrum/
├── CMakeLists.txt
├── pico_sdk_import.cmake
└── src/
    ├── main.c              # Application entry point
    ├── adc_mcp3202.c/h     # SPI ADC + DMA
    ├── dsp.c/h             # Fixed-point FFT + band extraction
    ├── dsp_time.c/h        # Time-domain audio effects
    ├── audio_out_pwm.c/h   # PWM audio output (DMA)
    ├── display.c/h         # I2C LED display functions
    ├── ht16k33.c/h         # Lower-level 16×16 HT16K33 LED display driver
    └── debug_usb.c/h       # USB debug & control
```


## Building the Project

Prerequisites
* Pico SDK installed
* CMake ≥ 3.13
* ARM GCC toolchain

Build Steps: 

```
git clone <your-repo-url>
cd pico-spectrum
mkdir build
cd build
cmake ..
make -j
```

Flash to Pico

Hold BOOTSEL button, connect USB, then:
```
cp pico_spectrum.uf2 /media/<your-pico>
```

📜 License

MIT License — free to use, modify, and redistribute.