#pragma once
#include <stdint.h>
#include "pico/stdlib.h"

void audio_pwm_init(uint gpio);
void audio_pwm_play(const int16_t *samples);
