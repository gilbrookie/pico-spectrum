#pragma once
#include <stdint.h>

void adc_init(void);
void adc_start_dma(void);

extern volatile int16_t *adc_ready_buffer;
