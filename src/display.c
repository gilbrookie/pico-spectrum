#include "display.h"
#include "ht16k33.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include <string.h>
#include <math.h>  // for fminf/fmaxf

// --- Framebuffers for 4 daisy-chained 8x8 modules ---
static uint8_t fb0[8], fb1[8], fb2[8], fb3[8];

// Column map to accommodate the LED backpack module
static const uint8_t col_map_8[8] = {7,0,1,2,3,4,5,6};

// default brightness value
static uint8_t global_brightness = 15;

// --- Framebuffer helpers ---
void display_clear(void) {
    memset(fb0, 0, sizeof(fb0));
    memset(fb1, 0, sizeof(fb1));
    memset(fb2, 0, sizeof(fb2));
    memset(fb3, 0, sizeof(fb3));
}

/*
Assumes wiring of the four 8×8 modules in this orientation:

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
*/


// Logical (0..15,0..15) → physical LED
void display_set_pixel(int x, int y) {
    if ((unsigned)x > 15 || (unsigned)y > 15) return;

    uint8_t bit = col_map_8[x & 7];

    if (y < 8) {
        if (x < 8) fb0[y] |= (1 << bit);
        else       fb1[y] |= (1 << bit);
    } else {
        if (x < 8) fb2[y - 8] |= (1 << bit);
        else       fb3[y - 8] |= (1 << bit);
    }
}

// Initialize the 12c channel for each 8x8 LED grid
void display_init(void) {
    // I2C setup
    i2c_init(DISPLAY_I2C_PORT, DISPLAY_I2C_BAUD);
    gpio_set_function(DISPLAY_I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(DISPLAY_I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(DISPLAY_I2C_SDA);
    gpio_pull_up(DISPLAY_I2C_SCL);
    sleep_ms(10);

    // Initialize HT16K33 devices
    ht16k33_init(HT16K33_ADDR0);
    ht16k33_init(HT16K33_ADDR1);
    ht16k33_init(HT16K33_ADDR2);
    ht16k33_init(HT16K33_ADDR3);

    display_clear();
    display_render();
}


// --- Render framebuffer to physical LEDs ---
void display_render(void) {
    ht16k33_update( HT16K33_ADDR0, fb0);
    ht16k33_update( HT16K33_ADDR1, fb1);
    ht16k33_update( HT16K33_ADDR2, fb2);
    ht16k33_update( HT16K33_ADDR3, fb3);
}

static void display_set_brightness(uint8_t level) {
    ht16k33_set_brightness(HT16K33_ADDR0, level);
    ht16k33_set_brightness(HT16K33_ADDR1, level);
    ht16k33_set_brightness(HT16K33_ADDR2, level);
    ht16k33_set_brightness(HT16K33_ADDR3, level);
}


// --------------------------------------------------

static void test_line(void) {
    static int y = 0;

    display_clear();
    for (int x = 0; x < 16; x++)
        display_set_pixel(x, y);

    y = (y + 1) % 16;
}

static void test_column(void) {
    static int x = 0;

    display_clear();
    for (int y = 0; y < 16; y++)
        display_set_pixel(x, y);

    x = (x + 1) % 16;
}

static void test_pixel(void) {
    static int x = 0, y = 0;

    display_clear();
    display_set_pixel(x, y);

    x++;
    if (x >= 16) {
        x = 0;
        y = (y + 1) % 16;
    }
}

static void test_checkerboard(void) {
    display_clear();
    for (int y = 0; y < 16; y++)
        for (int x = 0; x < 16; x++)
            if ((x ^ y) & 1)
                display_set_pixel(x, y);
}


static void spectrum_draw(const uint8_t *spectrum) {
    display_clear();
    if (!spectrum) return;

    for (int x = 0; x < 16; x++) {
        uint8_t h = spectrum[x];
        if (h > 16) h = 16;
        for (int y = 0; y < h; y++)
            display_set_pixel(x, y);
    }
}

static void test_brightness(void) {
    static int dir = -1;
    static int counter = 0;
    display_clear();

    // Light entire display so brightness is obvious
    for (int y = 0; y < 16; y++)
        for (int x = 0; x < 16; x++)
            display_set_pixel(x, y);

    display_set_brightness(global_brightness);

    // Only change brightness every N frames
    counter++;
    if (counter >= 5) {   // change every 5 frames
        counter = 0;
        global_brightness += dir;
        if (global_brightness == 0 || global_brightness == 15)
            dir = -dir;
    }

}


// --------------------------------------------------

void display_update(display_mode_t mode, const uint8_t *spectrum) {
    switch (mode) {
        case DISPLAY_TEST_LINE:         test_line();        break;
        case DISPLAY_TEST_COLUMN:       test_column();      break;
        case DISPLAY_TEST_CHECKERBOARD: test_checkerboard();break;
        case DISPLAY_TEST_PIXEL:        test_pixel();       break;
        case DISPLAY_SPECTRUM:          spectrum_draw(spectrum); break;
        case DISPLAY_TEST_BRIGHTNESS:
            test_brightness();
            break;
        default: break;
    }
}



void display_update_float(const float *spectrum, int length) {
    if (!spectrum || length <= 0) return;

    display_clear();

    // Find max value in the array for normalization
    float max_val = 0.0f;
    for (int i = 0; i < length; i++) {
        if (spectrum[i] > max_val) max_val = spectrum[i];
    }
    if (max_val <= 0.0f) max_val = 1.0f;  // avoid divide by zero

    // Map to LED columns
    for (int col = 0; col < LED_COLUMNS; col++) {
        // Map input spectrum index to LED column
        int start = col * length / LED_COLUMNS;
        int end   = (col + 1) * length / LED_COLUMNS;
        if (end > length) end = length;

        // Average the values in this range
        float sum = 0.0f;
        for (int i = start; i < end; i++) sum += spectrum[i];
        float avg = sum / (end - start);

        // Map average to 0–LED_HEIGHT
        uint8_t h = (uint8_t)fminf((avg / max_val) * LED_HEIGHT, LED_HEIGHT);

        // Light up the LEDs
        for (int y = 0; y < h; y++) {
            display_set_pixel(col, y);
        }
    }
}