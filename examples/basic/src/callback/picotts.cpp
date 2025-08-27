#include "app.h"

void picotts_output_callback(int16_t *samples, unsigned count) {
    for (unsigned i = 0; i < count; i++) {
        int32_t amplified = (int32_t)(samples[i] * volume_multiplier);
        
        if (amplified > 32767) amplified = 32767;
        if (amplified < -32768) amplified = -32768;
        
        int16_t boosted_sample = (int16_t)amplified;
        
        collected_audio.push_back(boosted_sample);
    }
    vTaskDelay(pdMS_TO_TICKS(1));
}

void picotts_error_callback(void) {
    ESP_LOGE(TAG_PICO_CALLBACK, "PicoTTS engine encountered an error and stopped");
}

void picotts_idle_callback(void) {
    ESP_LOGD(TAG_PICO_CALLBACK, "PicoTTS engine is now idle");
    
    if (!collected_audio.empty() && i2sSpeaker) {
        std::vector<int16_t> speed_adjusted_audio = apply_speed_adjustment(collected_audio);
        i2sSpeaker->writeSamples(speed_adjusted_audio.data(), speed_adjusted_audio.size());
        ESP_LOGD(TAG_PICO_CALLBACK, "Played speed-adjusted audio samples to speaker (speed: %.2f)", playback_speed);
        
        collected_audio.clear();
    }
}