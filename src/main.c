#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "adc_mcp3202.h"
#include "dsp.h"
#include "dsp_time.h"
#include "audio_out_pwm.h"
#include "display.h"
#include "debug_usb.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static int16_t audio_out[256];
static float mix = 0.7f;
static bool bypass = false;



// Run this on Core 1
void core1_entry() {
    dsp_init();
    dsp_time_init();
    audio_pwm_init(15);

    while (1) {
        if (adc_ready_buffer) {
            dsp_process(adc_ready_buffer);
            dsp_time_process(adc_ready_buffer, audio_out, mix, bypass);
            audio_pwm_play(audio_out);
            adc_ready_buffer = NULL;
        }
        tight_loop_contents();
    }
}

int main() {

    stdio_init_all();
    srand(time(NULL));   // seed random generator
    sleep_ms(1500);

    display_init();

#ifdef DISPLAY_TEST
    // run the display sequence for testing purposes
    display_test();
#else
    // main app loop
    adc_init();
    adc_start_dma();

    multicore_launch_core1(core1_entry);

    while (1) {
        display_update_float(band_levels, 16);
        display_render();
        debug_print_bands(band_levels);

        int c = getchar_timeout_us(0);
        if (c != PICO_ERROR_TIMEOUT)
            debug_handle_cmd(c, &mix, &bypass);

        sleep_ms(30);
    }
#endif
    return 0;
}
