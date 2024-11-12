#include "driver/dac.h"
#include "soc/dac_channel.h"
#include "soc/dport_reg.h"
#include "driver/periph_ctrl.h"
#include "esp32-hal.h"
#include "esp32-hal-dma.h"

#define SAMPLE_RATE 16000  // 16 KHz
#define TONE_FREQ 1000     // 1 KHz tone
#define DAC_CHANNEL DAC_CHANNEL_1  // DAC1 on GPIO25

const int SAMPLE_COUNT = SAMPLE_RATE / TONE_FREQ;  // Number of samples per wave cycle
uint8_t sineWave[SAMPLE_COUNT];  // Array to store the sine wave values

// DMA descriptor
dma_descriptor_t dmaDesc;

// Initialize the sine wave buffer
void initSineWave() {
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    float angle = (float)i / SAMPLE_COUNT * 2.0 * PI;
    sineWave[i] = (uint8_t)(127.5 * (sin(angle) + 1));  // 8-bit values (0-255)
  }
}

void setup() {
  // Initialize DAC output
  dac_output_enable(DAC_CHANNEL);

  // Initialize the sine wave data
  initSineWave();

  // Set up DMA
  setupDMA();
}

void loop() {
  // Nothing needed here as DMA is handling playback
}

void setupDMA() {
  // Configure the DMA descriptor
  dmaDesc.buffer = sineWave;                  // Point to the sine wave data
  dmaDesc.size = SAMPLE_COUNT;                // Set the size of the data
  dmaDesc.length = SAMPLE_COUNT;              // Length of the sine wave data
  dmaDesc.eof = 1;                            // Set EOF (end of frame)
  dmaDesc.sosf = 0;                           // Start of subframe flag
  dmaDesc.owner = 1;                          // DMA owns this descriptor
  dmaDesc.empty = 0;                          // Not an empty buffer
  dmaDesc.next = &dmaDesc;                    // Loop back to itself for continuous playback

  // Initialize the DAC DMA peripheral
  DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, DPORT_I2S0_CLK_EN);
  DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_I2S0_RST);

  // Configure I2S to work with the DAC
  i2s_dev_t* i2s = &I2S0;
  i2s->conf.tx_start = 0;                     // Stop I2S TX to reset
  i2s->conf.tx_mono = 0;                      // Stereo mode (single channel)
  i2s->conf.tx_slave_mod = 1;                 // Set as master mode
  i2s->sample_rate_conf.tx_bck_div_num = 1;   // Set bit clock
  i2s->sample_rate_conf.tx_bits_mod = 8;      // Set bits per sample to 8
  i2s->conf1.tx_pcm_bypass = 1;               // Bypass PCM to DAC directly

  // DMA buffer address and sample frequency configuration
  i2s->out_link.addr = (uint32_t)(&dmaDesc);  // Link DMA descriptor
  i2s->out_link.start = 1;                    // Start the DMA out link
  i2s->conf.tx_start = 1;                     // Start I2S TX

  // Set the sample rate
  i2s->clkm_conf.clkm_div_a = 1;
  i2s->clkm_conf.clkm_div_b = 0;
  i2s->clkm_conf.clkm_div_num = 2;
  i2s->sample_rate_conf.tx_bits_mod = 8;      // 8-bit samples
  i2s->sample_rate_conf.tx_bck_div_num = 8;   // Set for 16 KHz output
}
