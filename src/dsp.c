#include "dsp.h"
#include <string.h>
#include <math.h>

#define FFT_SIZE   256
#define NUM_BANDS  16

/* ---------- Fixed-point FFT types ---------- */

typedef struct {
    int16_t re;  // real
    int16_t im;  // imaginary
} cpx16_t;       // complex number 

static cpx16_t fft_buf[FFT_SIZE];

/* Output bands */
float band_levels[NUM_BANDS];

/* ---------- Sine table (Q15) ---------- */

static const int16_t sin_lut[128] = {
      0,   804,  1608,  2410,  3212,  4011,  4808,  5602,
   6393,  7179,  7962,  8739,  9512, 10278, 11039, 11793,
  12539, 13279, 14010, 14732, 15446, 16151, 16846, 17530,
  18204, 18868, 19519, 20159, 20787, 21403, 22005, 22594,
  23170, 23731, 24279, 24811, 25329, 25832, 26319, 26790,
  27245, 27683, 28105, 28510, 28898, 29268, 29621, 29956,
  30273, 30571, 30852, 31113, 31356, 31580, 31785, 31971,
  32138, 32286, 32415, 32524, 32615, 32686, 32738, 32767,
};

/* ---------- Helpers ---------- */

static inline int16_t q15_mul(int16_t a, int16_t b) {
    return (int16_t)((a * b) >> 15);
}

static inline int16_t sin_q15(int i) {
    return sin_lut[i & 0x7F];
}

static inline int16_t cos_q15(int i) {
    return sin_lut[(i + 32) & 0x7F];
}

/* ---------- FFT ---------- */

static void fft256(cpx16_t *buf) {
    /* Bit reversal */
    for (uint16_t i = 1, j = 0; i < FFT_SIZE; i++) {
        uint16_t bit = FFT_SIZE >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j |= bit;
        if (i < j) {
            cpx16_t t = buf[i];
            buf[i] = buf[j];
            buf[j] = t;
        }
    }

    /* FFT Butterfly */
    for (uint16_t len = 2; len <= FFT_SIZE; len <<= 1) {
        // number of butterflies per group
        uint16_t half = len >> 1;
        // twiddle factor spacing
        uint16_t step = FFT_SIZE / len;

        // iterate over each group
        for (uint16_t i = 0; i < FFT_SIZE; i += len) {
            // iterate over the butterflies in each group
            for (uint16_t j = 0; j < half; j++) {
                // index into the SIN/COS table
                uint16_t k = j * step;
                // twiddle factor
                // W=cos−j⋅sin
                int16_t wr = cos_q15(k);
                int16_t wi = -sin_q15(k);

                cpx16_t a = buf[i + j];
                cpx16_t b = buf[i + j + half];
                
                int16_t tr =
                    q15_mul(b.re, wr) - q15_mul(b.im, wi);
                int16_t ti =
                    q15_mul(b.re, wi) + q15_mul(b.im, wr);

                buf[i + j].re = a.re + tr;
                buf[i + j].im = a.im + ti;
                buf[i + j + half].re = a.re - tr;
                buf[i + j + half].im = a.im - ti;
            }
        }
    }
}

/* ---------- Public API ---------- */

void dsp_init(void) {
    memset(band_levels, 0, sizeof(band_levels));
}

// 0.6f is a visual tuning constant for log compressions
const float VISUAL_TUNING = 0.6f;

void dsp_process(volatile int16_t *samples) {
    /* Copy input (Q15) */
    for (int i = 0; i < FFT_SIZE; i++) {
        fft_buf[i].re = samples[i] << 3;  // scale 12-bit ADC to ~15 bit range
        fft_buf[i].im = 0;
    }

    fft256(fft_buf);

    /* Clear bands */
    for (int b = 0; b < NUM_BANDS; b++)
        band_levels[b] = 0;

    /* Bin → band mapping (log-ish) */
    for (int i = 1; i < FFT_SIZE / 2; i++) {
        int band = (i * NUM_BANDS) / (FFT_SIZE / 2);
        int32_t mag =
            (int32_t)fft_buf[i].re * fft_buf[i].re +
            (int32_t)fft_buf[i].im * fft_buf[i].im;

        band_levels[band] += (float)mag;
    }


    /* Log compression + scaling */
    for (int b = 0; b < NUM_BANDS; b++) {
        float v = band_levels[b];
        band_levels[b] = v > 0 ? (log10f(v) * VISUAL_TUNING) : 0;
    }
}
