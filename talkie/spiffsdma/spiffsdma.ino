#include "driver/i2s.h"
#include "SPIFFS.h"

#define SAMPLE_RATE 16000
#define BUFFER_SIZE 1024  // Size of each DMA buffer
#define I2S_NUM I2S_NUM_0
#define DAC_PIN 25        // DAC1 (GPIO25)
#define WAV_FILE "/test.wav"  // Path to your WAV file in SPIFFS

// Number of DMA buffers
#define MAX_DMA_BUFFERS 8

// DMA buffer array
uint8_t dma_buffers[MAX_DMA_BUFFERS][BUFFER_SIZE];

// I2S Configuration
i2s_config_t i2s_config = {
    .mode = I2S_MODE_TX,
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_8BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,  // Mono output to DAC
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = MAX_DMA_BUFFERS,
    .dma_buf_len = BUFFER_SIZE,
    .tx_desc_auto_clear = true
};

i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_PIN_NO_CHANGE,  // Not needed for DAC mode
    .ws_io_num = I2S_PIN_NO_CHANGE,   // Not needed for DAC mode
    .data_out_num = DAC_PIN,          // DAC1 pin (GPIO 25)
    .data_in_num = I2S_PIN_NO_CHANGE  // Not used
};

void setup() {
    // Start Serial for debugging
    Serial.begin(115200);

    // Initialize SPIFFS
    if (!SPIFFS.begin()) {
        Serial.println("Failed to mount SPIFFS");
        return;
    }

    // Open the WAV file
    File wavFile = SPIFFS.open(WAV_FILE, "r");
    if (!wavFile) {
        Serial.println("Failed to open WAV file");
        return;
    }

    // Install I2S driver
    ESP_ERROR_CHECK(i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL));

    // Set I2S pin configuration for DAC
    ESP_ERROR_CHECK(i2s_set_pin(I2S_NUM, &pin_config));

    size_t bytesRead = 0;
    size_t bytesWritten = 0;

    // Stream audio file data from SPIFFS to I2S using DMA buffers
    while (wavFile.available()) {
        for (int i = 0; i < MAX_DMA_BUFFERS; i++) {
            // Read a chunk of the file into the DMA buffer
            bytesRead = wavFile.read(dma_buffers[i], BUFFER_SIZE);
            if (bytesRead == 0) {
                break;  // No more data to read from the file
            }

            // Write the buffer to the I2S peripheral
            esp_err_t result = i2s_write(I2S_NUM, (const char*)dma_buffers[i], bytesRead, &bytesWritten, portMAX_DELAY);
            if (result == ESP_OK) {
                Serial.printf("Wrote %d bytes to I2S\n", bytesWritten);
            } else {
                Serial.println("I2S write failed");
                break;
            }
        }
    }

    wavFile.close();
}

void loop() {
    // No need to loop, playback is done in setup()
}
