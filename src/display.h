#pragma once
#include <stdint.h>

// ---- I2C configuration (DISPLAY OWNS THIS) ----
#define DISPLAY_I2C_PORT i2c0
#define DISPLAY_I2C_SDA  4
#define DISPLAY_I2C_SCL  5
#define DISPLAY_I2C_BAUD 400000

// I2C addresses for the four HT16K33 backpacks
#define HT16K33_ADDR0 0x70
#define HT16K33_ADDR1 0x71
#define HT16K33_ADDR2 0x72
#define HT16K33_ADDR3 0x73

#define LED_HEIGHT 16
#define LED_COLUMNS 16

#define FONT_W 8
#define FONT_H 8

typedef enum {
    DISPLAY_TEST_LINE,
    DISPLAY_TEST_COLUMN,
    DISPLAY_TEST_CHECKERBOARD,
    DISPLAY_TEST_PIXEL,
    DISPLAY_TEST_BRIGHTNESS,
    DISPLAY_SPECTRUM,
    DISPLAY_VU,
    DISPLAY_CHAR,
} display_mode_t;

// initialize the display
void display_init(void);

// Update framebuffer only (no I2C writes)
void display_update(display_mode_t mode, const uint8_t *spectrum);

// Update the framebuffer using an array of float values
void display_update_float(const float *spectrum, int length);

// Push framebuffer to all HT16K33 devices
void display_render(void);

// Optional helpers (useful elsewhere)
void display_clear(void);
void display_set_pixel(int x, int y);

void display_test(void);
