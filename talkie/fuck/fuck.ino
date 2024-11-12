#include "driver/i2s.h"
#include "SPIFFS.h"

File wavFile;

// Constants for audio output
#define SAMPLE_RATE 16000
#define BUFFER_SIZE 1024 // buffer size for audio data
#define I2S_NUM I2S_NUM_0
#define DAC_PIN 25       // DAC1 (GPIO25)

// DMA buffer for I2S data
uint8_t i2s_buffer[BUFFER_SIZE];

// I2S configuration
i2s_config_t i2s_config = {
    .mode = I2S_MODE_TX,                      // Transmit mode
    .sample_rate = SAMPLE_RATE,               // Sample rate
    .bits_per_sample = I2S_BITS_PER_SAMPLE_8BIT, // 8 bits per sample
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // Mono audio output (only left channel)
    .communication_format = I2S_COMM_FORMAT_I2S, // I2S standard format
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,  // Interrupt level
    .dma_buf_count = 8,                       // Number of DMA buffers
    .dma_buf_len = BUFFER_SIZE,               // Length of each DMA buffer
    .tx_desc_auto_clear = true                // Auto-clear TX descriptors
};

i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_PIN_NO_CHANGE,  // No need for BCK pin in DAC mode
    .ws_io_num = I2S_PIN_NO_CHANGE,   // No need for WS pin in DAC mode
    .data_out_num = DAC_PIN,          // DAC pin for output
    .data_in_num = I2S_PIN_NO_CHANGE  // Not used in DAC mode
};

// Setup for I2S with DMA
void setup() {
  Serial.begin(115200);

  // Install the I2S driver
  ESP_ERROR_CHECK(i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL));

  // Set the I2S pin configuration (output to DAC)
  ESP_ERROR_CHECK(i2s_set_pin(I2S_NUM, &pin_config));

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS mount failed");
    return;
  }

  // Open the WAV file from SPIFFS
  wavFile = SPIFFS.open("/ceilings16.wav", "r");
  if (!wavFile) {
    Serial.println("Failed to open file");
    return;
  }

  Serial.printf("File size: %d bytes\n", wavFile.size());
}

void loop() {
  while (wavFile.available()) {
    // Read the next chunk of audio data into the buffer
    size_t bytesRead = wavFile.read(i2s_buffer, BUFFER_SIZE);

    // Write the buffer to the I2S peripheral for DMA transfer
    size_t bytesWritten;
    esp_err_t result = i2s_write(I2S_NUM, (const char*)i2s_buffer, bytesRead, &bytesWritten, portMAX_DELAY);
    if (result != ESP_OK) {
      Serial.println("I2S write failed");
    }

    // Optional: Add a small delay for pacing the audio playback (or use a timer interrupt)
    delay(10);
  }

  // Close the WAV file after playback
  wavFile.close();
  Serial.println("Finished playback");
}
