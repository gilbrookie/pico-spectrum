#pragma once
#include <stdint.h>

void ht16k33_init(uint8_t addr);
void ht16k33_update(uint8_t addr, const uint8_t *rows);
void ht16k33_set_brightness(uint8_t addr, uint8_t level);