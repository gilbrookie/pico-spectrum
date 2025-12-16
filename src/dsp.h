#pragma once
#include <stdint.h>

void dsp_init(void);
void dsp_process(volatile int16_t *samples);

extern float band_levels[16];
