#include "app.h"

void setupPicoTTS() {
    ESP_LOGI(TAG_PICO, "Setting up PicoTTS...");
    if (!i2sSpeaker) {
        ESP_LOGE(TAG_PICO, "Cannot setup PicoTTS: I2S speaker not initialized");
        return;
    }
    
    if (picotts_init(PICOTTS_TASK_PRIORITY, picotts_output_callback, PICOTTS_CORE)) {        
        picotts_set_error_notify(picotts_error_callback);
        picotts_set_idle_notify(picotts_idle_callback);

        sayText("Hi, I am cozmo. Nice to meet you.");
    } else {
        ESP_LOGE(TAG_PICO, "Failed to initialize PicoTTS engine");
    }
}

std::vector<int16_t> apply_speed_adjustment(const std::vector<int16_t>& samples) {
    if (playback_speed == 1.0f) {
        return samples; // No speed change needed
    }
    
    std::vector<int16_t> adjusted_samples;
    size_t output_size = (size_t)(samples.size() / playback_speed);
    adjusted_samples.reserve(output_size);
    
    for (size_t i = 0; i < output_size; i += 2) {
        float source_index = i * playback_speed;
        size_t index = (size_t)source_index;
        
        if (index + 1 < samples.size()) {
            adjusted_samples.push_back(samples[index]);
        }
    }
    
    return adjusted_samples;
}

bool sayText(const char* text) {
    // Validate text length
    if (strlen(text) > PICOTTS_MAX_TEXT_LENGTH) {
        ESP_LOGW(TAG_PICO, "Text too long (%d chars), truncating to %d", 
            strlen(text), PICOTTS_MAX_TEXT_LENGTH);
            
        char* truncated_text = (char*)malloc(PICOTTS_MAX_TEXT_LENGTH + 1);
        strncpy(truncated_text, text, PICOTTS_MAX_TEXT_LENGTH);
        truncated_text[PICOTTS_MAX_TEXT_LENGTH] = '\0';
        text = truncated_text;
    }

    ESP_LOGI(TAG_PICO, "Task says: %s", text); 
    const size_t length = strlen(text) + 1;
    char arr[length]; 
    strcpy(arr, text);

    picotts_add(arr, sizeof(arr));
    return true;
}
