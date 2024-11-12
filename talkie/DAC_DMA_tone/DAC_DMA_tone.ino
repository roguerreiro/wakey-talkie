#include <Arduino.h>
#include "driver/i2s.h"

#define SAMPLE_RATE 16000   // 16 kHz sample rate
#define BUFFER_SIZE 1024    // DMA buffer size (in bytes)
#define I2S_NUM I2S_NUM_0   // I2S peripheral 0
#define DAC_PIN 25          // DAC1 (GPIO 25)

uint8_t buf1[BUFFER_SIZE];  
uint8_t buf2[BUFFER_SIZE];

volatile bool buffer_in_use = false;

// I2S configuration
i2s_config_t i2s_config = {
    .mode = I2S_MODE_TX,                         // TX mode (Transmit)
    .sample_rate = SAMPLE_RATE,                  // Sample rate
    .bits_per_sample = I2S_BITS_PER_SAMPLE_8BIT, // 8-bit samples
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // Mono output to DAC
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,    // Interrupt flags
    .dma_buf_count = 2,                          // At least 2 buffers
    .dma_buf_len = BUFFER_SIZE,                  // DMA buffer length
    .tx_desc_auto_clear = true                   // Auto-clear TX descriptors
};

// Pin configuration for DAC
i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_PIN_NO_CHANGE,  // Not needed for basic DAC
    .ws_io_num = I2S_PIN_NO_CHANGE,   // Not needed for basic DAC
    .data_out_num = DAC_PIN,          // DAC data pin (GPIO 25)
    .data_in_num = I2S_PIN_NO_CHANGE  // Not used in TX mode
};

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Initialize I2S with DMA
    esp_err_t err = i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.println("I2S driver installation failed!");
        return;
    }

    err = i2s_set_pin(I2S_NUM, &pin_config);
    if (err != ESP_OK) {
        Serial.println("I2S pin configuration failed!");
        return;
    }

    Serial.println("Test tone generation started.");
}

void fillBuffer(int bufferNumber)
{
  if(bufferNumber == 1)
  {
    for(int i=0; i<BUFFER_SIZE; i++)
    {
      buf1[i] = i;
    }
  }
  else if(bufferNumber == 2)
  {
    for(int i=0; i<BUFFER_SIZE; i++)
    {
      buf2[i] = 255 - i;
    }
  }
}

void writeBuffer(int bufferNumber)
{
  size_t bytesWritten;
  if(bufferNumber == 1)
  {
    i2s_write(I2S_NUM, (const char*)buf1, BUFFER_SIZE, &bytesWritten, portMAX_DELAY);
  }
  else if(bufferNumber == 2)
  {
    i2s_write(I2S_NUM, (const char*)buf2, BUFFER_SIZE, &bytesWritten, portMAX_DELAY);
  }
}

void loop() 
{
    if (!buffer_in_use) {
        fillBuffer(1);      // Fill buffer 1 with new data
        writeBuffer(1);     // Send buffer 1 to DAC
        buffer_in_use = true;  // Mark buffer as in use
    } else {
        fillBuffer(2);      // Fill buffer 2 with new data
        writeBuffer(2);     // Send buffer 2 to DAC
        buffer_in_use = false; // Mark buffer as in use
    }
}
