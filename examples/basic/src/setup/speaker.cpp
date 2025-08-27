#include "app.h"

I2SSpeaker* i2sSpeaker = nullptr;

void setupSpeakers() {
  ESP_LOGI(TAG_SPEAKER, "Setting up speakers...");
  i2sSpeaker = new I2SSpeaker(
    I2S_SPEAKER_DATA_PIN,
    I2S_SPEAKER_BCLK_PIN, 
    I2S_SPEAKER_WCLK_PIN
  );
  
  if (i2sSpeaker->init(I2S_SPEAKER_SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO) == ESP_OK) {
    ESP_LOGI(TAG_SPEAKER, "I2S speaker (MAX98357) initialized successfully");
    i2sSpeaker->start();
  } else {
    ESP_LOGE(TAG_SPEAKER, "I2S speaker (MAX98357) initialization failed");
    delete i2sSpeaker;
    i2sSpeaker = nullptr;
  }
}