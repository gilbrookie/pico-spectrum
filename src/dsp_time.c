#include "dsp_time.h"
#include <math.h>

#define FFT_SIZE 256

static float lp = 0;
static float gain = 1.2f;

void dsp_time_init() { lp = 0; }

static inline float soft_clip(float x) {
    return x / (1.0f + fabsf(x));
}

void dsp_time_process(
    volatile int16_t *in,
    int16_t *out,
    float mix,
    bool bypass
) {
    for (int i = 0; i < FFT_SIZE; i++) {
        float dry = in[i];
        if (bypass) { out[i] = in[i]; continue; }

        float wet = soft_clip((lp += 0.15f * (dry*gain - lp)));
        float v = dry * (1 - mix) + wet * mix;

        if (v > 2047) v = 2047;
        if (v < -2048) v = -2048;
        out[i] = (int16_t)v;
    }
}
