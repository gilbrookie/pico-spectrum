#include "ht16k33.h"
#include "display.h"
#include "hardware/i2c.h"
#include <stdint.h>


void ht16k33_init(uint8_t addr) {
    uint8_t cmd;

    cmd = 0x21; // oscillator on
    i2c_write_blocking(DISPLAY_I2C_PORT, addr, &cmd, 1, false);

    cmd = 0x81; // display on, blink off
    i2c_write_blocking(DISPLAY_I2C_PORT, addr, &cmd, 1, false);

    cmd = 0xEF; // brightness max
    i2c_write_blocking(DISPLAY_I2C_PORT, addr, &cmd, 1, false);
}

void ht16k33_update(uint8_t addr, const uint8_t *rows) {
    uint8_t buffer[17];

    buffer[0] = 0x00; // RAM start address

    for (int i = 0; i < 8; i++) {
        buffer[1 + i * 2]     = rows[i];
        buffer[1 + i * 2 + 1] = 0x00; // unused
    }

    i2c_write_blocking(
        DISPLAY_I2C_PORT,
        addr,
        buffer,
        sizeof(buffer),
        false
    );
}

void ht16k33_set_brightness(uint8_t addr, uint8_t level) {
    if (level > 15) level = 15;
    uint8_t cmd = 0xE0 | level;
    i2c_write_blocking(DISPLAY_I2C_PORT, addr, &cmd, 1, false);
}
