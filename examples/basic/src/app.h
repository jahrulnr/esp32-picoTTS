#pragma once

#include <Arduino.h>
#include <esp_log.h>
#include <esp_task_wdt.h>
#include <vector>
#include "picotts.h"
#include <I2SSpeaker.h>

static const char* TAG_SPEAKER = "Speaker";
static const char* TAG_PICO = "picoTTS";
static const char* TAG_PICO_CALLBACK = "picoTTS_Callback";

static float playback_speed = 1.0f;
static const float volume_multiplier = 1.0f;
static std::vector<int16_t> collected_audio;

#define PICOTTS_TASK_PRIORITY 10
#define PICOTTS_CORE 1
#define PICOTTS_MAX_TEXT_LENGTH 512

#define I2S_SPEAKER_DATA_PIN GPIO_NUM_13
#define I2S_SPEAKER_BCLK_PIN GPIO_NUM_12
#define I2S_SPEAKER_WCLK_PIN GPIO_NUM_11
#define I2S_SPEAKER_SAMPLE_RATE 16000

extern I2SSpeaker* i2sSpeaker;
void setupSpeakers();
void setupPicoTTS();

std::vector<int16_t> apply_speed_adjustment(const std::vector<int16_t>& samples);
bool sayText(const char* text);

void picotts_output_callback(int16_t *samples, unsigned count);
void picotts_error_callback(void);
void picotts_idle_callback(void);

