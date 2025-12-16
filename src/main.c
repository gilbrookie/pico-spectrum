#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "adc_mcp3202.h"
#include "dsp.h"
#include "dsp_time.h"
#include "audio_out_pwm.h"
#include "display.h"
#include "debug_usb.h"

#include <stdio.h>

static int16_t audio_out[256];
static float mix = 0.7f;
static bool bypass = false;

// A struct to capture LED display modes
typedef struct {
    display_mode_t mode;
    int frames;          // how many updates until done
} display_test_t;


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
    sleep_ms(1500);

    display_init();

    printf("Display test sequence start\n");

    display_init();

    // each test will run for a specified number of frames
    display_test_t tests[] = {
        { DISPLAY_TEST_LINE,        16  },
        { DISPLAY_TEST_COLUMN,      16  },
        { DISPLAY_TEST_PIXEL,       256 },
        { DISPLAY_TEST_CHECKERBOARD, 32 }, 
        { DISPLAY_TEST_BRIGHTNESS,  182  },
    };

    const int test_count = sizeof(tests) / sizeof(tests[0]);
    int test_index = 0;
    int frame = 0;

    while (true) {
        display_update(tests[test_index].mode, NULL);
        display_render();

        frame++;
        if (frame >= tests[test_index].frames) {
            frame = 0;
            test_index = (test_index + 1) % test_count;
            printf("Switching to test %d\n", test_index);
        }

        sleep_ms(50);  // animation speed
    }


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
}
