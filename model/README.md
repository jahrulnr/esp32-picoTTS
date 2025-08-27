# PicoTTS Language Models

This directory contains the pre-trained language models required for the ESP32-picoTTS library to function.

## 📁 Model Files

### `en-US_ta.bin` - Text Analysis Model
- **Size**: ~640KB
- **Function**: Handles text analysis, phoneme mapping, and linguistic processing
- **Partition**: `picotts_ta` 
- **Flash Address**: `0x610000`

### `en-US_lh0_sg.bin` - Signal Generation Model  
- **Size**: ~820KB
- **Function**: Converts phonemes to audio signals and waveform synthesis
- **Partition**: `picotts_sg`
- **Flash Address**: `0x6B0000`

## 🚀 Model Installation

### Automatic Installation (Recommended)

Use the partition manager tool included in the example project:

```bash
cd examples/basic
pio run --target upload --target uploadfs
```

### Manual Installation

Flash the models directly using esptool.py:

```bash
# Flash text analysis model
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 2000000 \
    --before default_reset --after hard_reset \
    write_flash 0x610000 en-US_ta.bin

# Flash signal generation model
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 2000000 \
    --before default_reset --after hard_reset \
    write_flash 0x6B0000 en-US_lh0_sg.bin
```

### Using PlatformIO Upload Tool

Add to your `platformio.ini`:

```ini
extra_scripts = 
    tools/partition_manager.py

platform_packages = 
    tool-esp32partitiontool@https://github.com/serifpersia/esp32partitiontool/releases/download/v1.4.5/esp32partitiontool-platformio.zip
```

## ⚙️ Partition Configuration

Ensure your partition table (`picoTTS.csv`) includes the required partitions:

```csv
# Name,      Type, SubType,  Offset,   Size,    Flags
picotts_ta,  data, undefined, 0x610000, 0xA0000,
picotts_sg,  data, undefined, 0x6B0000, 0xCD000,
```

## 🌍 Language Support

Currently supported languages:
- **English (US)**: `en-US` - Included models

Additional language models can be obtained from the original PicoTTS distribution and converted for ESP32 use. For more language models and conversion tools, check [github.com/DiUS/esp-picotts](https://github.com/DiUS/esp-picotts).

## 🔧 Memory Requirements

| Model | Flash Size | Description |
|-------|------------|-------------|
| Text Analysis | 640KB | Linguistic processing, phoneme mapping |
| Signal Generation | 820KB | Audio synthesis, waveform generation |
| **Total** | **1.46MB** | **Complete language pack** |

## ⚠️ Important Notes

1. **Model Dependencies**: Both models are required for the TTS engine to function
2. **Version Compatibility**: Models must match the PicoTTS engine version
3. **Endianness**: Models are little-endian formatted for ESP32
4. **Integrity**: Always verify checksums after flashing to ensure data integrity

## 🐛 Troubleshooting

### "Partition not found" Error
- Verify partition table includes `picotts_ta` and `picotts_sg` partitions
- Check partition sizes are sufficient (ta: ≥640KB, sg: ≥820KB)

### "Model load failed" Error  
- Ensure models are flashed to correct addresses
- Verify file integrity using read_flash verification
- Check for sufficient available flash memory

### "Resource corrupt" Error
- Re-flash the models with verified files
- Check for flash memory errors or bad sectors
- Ensure stable power supply during flashing

## 📖 References

- [Original PicoTTS Project](https://github.com/DiUS/esp-picotts)
- [ESP32 Partition Tables](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/partition-tables.html)
