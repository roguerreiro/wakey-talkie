#include "driver/i2s.h"
#include "esp_log.h"
#include "driver/i2s_std.h"

#include "SPIFFS.h"

File wavFile;

// Constants for audio output
#define SAMPLE_RATE 16000
#define BUFFER_SIZE 64 // maximum size 
#define I2S_NUM I2S_NUM_0
#define DAC_PIN 25  // DAC1 (GPIO25)

uint8_t i2s_buffer[BUFFER_SIZE];

// I2S configuration

i2s_config_t i2s_config = {
     .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
     .sample_rate = 44100,
     .bits_per_sample = I2S_BITS_PER_SAMPLE_8BIT, /* the DAC module will only take the 8bits from MSB */
     .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
     .communication_format = I2S_COMM_FORMAT_I2S_MSB,
     .intr_alloc_flags = 0, // default interrupt priority
     .dma_buf_count = 8,
     .dma_buf_len = 64,
     .use_apll = false
};
  
//    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),                      
//    .sample_rate = SAMPLE_RATE,               
//    .bits_per_sample = I2S_BITS_PER_SAMPLE_8BIT, // 8-bits per sample 
//    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // Mono output to DAC
//    .communication_format = I2S_COMM_FORMAT_I2S, // I2S standard format
//    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,  // Interrupt level
//    .dma_buf_count = 2,                       // Number of DMA buffers (linked list)
//    .dma_buf_len = BUFFER_SIZE,               // Size of each DMA buffer (in bytes)
//    .use_apll = false
//    .tx_desc_auto_clear = true                // Auto-clear TX descriptors after DMA transfer


//     .mode = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN,
//     .sample_rate = 16000,
//     .bits_per_sample = 16, /* the DAC module will only take the 8bits from MSB */
//     .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
//     .communication_format = I2S_COMM_FORMAT_I2S_MSB,
//     .intr_alloc_flags = 0, // default interrupt priority
//     .dma_buf_count = 8,
//     .dma_buf_len = 64,
//     .use_apll = false


//i2s_pin_config_t pin_config = {
//    .bck_io_num = I2S_PIN_NO_CHANGE,  // Not needed for DAC mode
//    .ws_io_num = I2S_PIN_NO_CHANGE,   // Not needed for DAC mode
//    .data_out_num = DAC_PIN,          // DAC1 pin (GPIO 25)
//    .data_in_num = I2S_PIN_NO_CHANGE  // Not used
//};

// Initialize I2S with DMA for DAC output
void setup() 
{
  Serial.begin(115200);
    // Install I2S driver
    ESP_ERROR_CHECK(i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL));
    
    // Set the I2S pin configuration for DAC
//    ESP_ERROR_CHECK(i2s_set_pin(I2S_NUM, &pin_config));

    i2s_set_pin(I2S_NUM, NULL);

//    i2s_set_dac_mode(I2S_DAC_CHANNEL_LEFT_EN);

    i2s_set_sample_rates(I2S_NUM, 22050); 

    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
      Serial.println("SPIFFS mount failed");
      delay(3000);
      return;
    }

    wavFile = SPIFFS.open("/pls.wav", "r");
    if (!wavFile) {
        Serial.println("Failed to open file");
        delay(3000);
        return;
    }

    Serial.printf("File size: %d bytes\n", wavFile.size());

    while (wavFile.available()) 
    {
          size_t bytesRead = wavFile.read(i2s_buffer, BUFFER_SIZE);
  
          size_t bytesWritten;
          esp_err_t result = i2s_write(I2S_NUM, (const char*)i2s_buffer, bytesRead, &bytesWritten, portMAX_DELAY);
    }
    Serial.println("Finished playback");
    wavFile.close();
}

void loop() 
{
  
}
