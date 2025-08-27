# ESP32-picoTTS

[![PlatformIO](https://img.shields.io/badge/PlatformIO-orange.svg)](https://platformio.org)
[![License: Apache 2.0](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![ESP32](https://img.shields.io/badge/ESP32-supported-green.svg)](https://www.espressif.com/en/products/socs/esp32)

**Turn your ESP32 into a talking device!** 🎤

This library brings high-quality text-to-speech capabilities to ESP32 microcontrollers using the proven PicoTTS engine. Simply send text and hear it spoken through I2S speakers.

## ✨ What This Does

Transform any ESP32 project into a voice-enabled device:
- **Text → Speech**: Convert any text string to natural-sounding voice
- **Plug & Play**: Works with common I2S audio modules (MAX98357A)
- **Memory Optimized**: Runs efficiently on ESP32 with 1.1MB RAM requirement
- **Multi-language Ready**: English included, more languages available

## 🚀 Quick Start

### 1. Copy the Example
The fastest way to get started is to copy our working example:

```bash
# Copy the entire examples/basic/ folder to your project
cp -r examples/basic/ my-tts-project/
cd my-tts-project/
```

### 2. Wire Your Speaker
Connect an I2S audio module (like MAX98357A):

```
ESP32 → MAX98357A
GPIO 13 → DIN
GPIO 12 → BCLK  
GPIO 11 → LRC
5V → VIN, GND → GND
```

### 3. Flash Models and Run
```bash
# First, flash the TTS models (see model/README.md for details)
cd model/
# Follow the model installation guide in model/README.md

# Then flash your code
cd ../examples/basic/
pio run --target upload
# Your ESP32 will now speak: "Hi, I am cozmo. Nice to meet you."
```

**⚠️ Important**: TTS models must be flashed separately! See [`model/README.md`](model/README.md) for complete model installation instructions.

## 💡 Basic Usage

```cpp
#include "picotts.h"

// Initialize TTS engine
picotts_init(10, audio_callback, 1);

// Make it speak!
const char* text = "Hello from ESP32!";
picotts_add(text, strlen(text));
```

## 📋 Requirements

- **ESP32** (any variant, ESP32-S3 recommended)
- **1.1MB RAM** for TTS engine
- **~1.7MB Flash** for English language models  
- **I2S Speaker** (MAX98357A or similar)

## 🛠️ Installation

### Option 1: PlatformIO Library
```bash
pio lib install https://github.com/jahrulnr/esp32-picoTTS.git
pio lib install https://github.com/jahrulnr/esp32-speaker.git
```

### Option 2: Copy Example Project
```bash
git clone https://github.com/jahrulnr/esp32-picoTTS.git
cd esp32-picoTTS/examples/basic/
pio run --target upload
```

## 🔧 How It Works

1. **Text Analysis**: Converts text to phonemes using linguistic models
2. **Speech Synthesis**: Generates audio waveforms from phonemes  
3. **Audio Output**: Streams 16kHz audio to I2S speaker
4. **Resource Management**: TTS models stored in ESP32 flash partitions

## 📁 Project Structure

```
esp32-picotts/
├── examples/basic/          ← Start here! Complete working example
├── src/                     ← Library source code
├── model/                   ← Language model files (en-US)
└── library.properties       ← Library metadata
```

## 🎯 Example Projects

- **Voice Announcements**: Weather updates, notifications
- **Interactive Devices**: Voice responses to sensors
- **Accessibility**: Audio feedback for visual displays
- **Educational**: Talking toys, learning devices

## 🐛 Troubleshooting

**No sound?**
- Check I2S wiring connections
- Verify models are flashed: `pio run --target upload`
- Ensure speaker is powered (5V for MAX98357A)

**Memory errors?**
- Use ESP32-S3 with PSRAM for best results
- Check available heap: `ESP.getFreeHeap()`

**For detailed documentation, see the full API reference in the source code.**

## 📄 License

Apache License 2.0 - Use freely in your projects!

## 🙏 Credits

- **[DiUS Computing](https://github.com/DiUS/esp-picotts)** - ESP32 port and original project
