#include "adc_mcp3202.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "pico/stdlib.h"

#define SPI_ADC spi0
#define PIN_CS  17
#define FFT_SIZE 256

static int16_t buffer_a[FFT_SIZE];
static int16_t buffer_b[FFT_SIZE];

volatile int16_t *adc_ready_buffer = NULL;
static bool use_a = true;
static int dma_chan;

// dma_handler is the interrupt routing when DMA channel finishes transferring FFT_SIZE samples
void __isr dma_handler() {
    // clear the DMA interrupt flag for the channel, so we don't immediately enter ISR again
    dma_hw->ints0 = 1u << dma_chan;
    adc_ready_buffer = use_a ? buffer_a : buffer_b;
    use_a = !use_a;

    dma_channel_set_write_addr(
        dma_chan,
        use_a ? buffer_a : buffer_b,
        true
    );
}

void adc_init() {
    spi_init(SPI_ADC, 2000000);
    spi_set_format(SPI_ADC, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);
    channel_config_set_dreq(&c, spi_get_dreq(SPI_ADC, false));

    dma_channel_configure(
        dma_chan,
        &c,
        buffer_a,
        &spi_get_hw(SPI_ADC)->dr,
        FFT_SIZE,
        false
    );

    dma_channel_set_irq0_enabled(dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);
}

void adc_start_dma() {
    dma_channel_start(dma_chan);
}
