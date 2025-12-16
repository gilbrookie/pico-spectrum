#pragma once
#include <stdint.h>
#include <stdbool.h>

void dsp_time_init(void);
void dsp_time_process(
    volatile int16_t *in,
    int16_t *out,
    float mix,
    bool bypass
);
