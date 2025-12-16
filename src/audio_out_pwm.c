#include "audio_out_pwm.h"
#include "hardware/pwm.h"
#include "hardware/dma.h"

#define FFT_SIZE 256
#define PWM_WRAP 499

static uint slice;
static int dma_chan;
static uint16_t pwm_buf[FFT_SIZE];

void audio_pwm_init(uint gpio) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    slice = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice, PWM_WRAP);
    pwm_set_enabled(slice, true);

    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_dreq(&c, DREQ_PWM_WRAP0 + slice);

    dma_channel_configure(
        dma_chan, &c,
        &pwm_hw->slice[slice].cc,
        pwm_buf,
        FFT_SIZE,
        false
    );
}

void audio_pwm_play(const int16_t *s) {
    for (int i = 0; i < FFT_SIZE; i++) {
        int v = s[i] + 2048;
        if (v < 0) v = 0;
        if (v > PWM_WRAP) v = PWM_WRAP;
        pwm_buf[i] = v;
    }
    dma_channel_set_read_addr(dma_chan, pwm_buf, true);
}
