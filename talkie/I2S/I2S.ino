#include "driver/i2s.h"

#define I2S_DATA_IN_PIN  34
#define I2S_DATA_OUT_PIN 25

void setup() {
    Serial.begin(115200);

    // Define the I2S configuration
    i2s_config_t i2s_config = {
        // Ensure we are using the correct type for the mode field
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),  // Cast explicitly to i2s_mode_t
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 1024
    };

    // Define pin configuration
    i2s_pin_config_t pin_config = {
        .bck_io_num = 26,
        .ws_io_num = 22,
        .data_out_num = I2S_DATA_OUT_PIN,
        .data_in_num = I2S_DATA_IN_PIN
    };

    // Install I2S driver
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
}

void loop() {
    // Placeholder for your loop logic
}
