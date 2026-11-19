# ESP32-S3 Real-Time Tuner

## Overview
This project implements a standalone, real-time chromatic guitar tuner built on the ESP32-S3 microcontroller. By combining hardware-level I2S audio acquisition with an embedded Digital Signal Processing (DSP) pipeline, the system captures acoustic signals via an INMP441 MEMS microphone, resolves fundamental frequencies with sub-Hz accuracy, and renders responsive visual tuning feedback on a dual-color 128x64 OLED display.

Designed around a modular C++ architecture (`audio_i2s`, `tuner_math`, `display_ui`, `main`) and strict static memory allocation, the firmware eliminates heap fragmentation and octave-jump errors common in naive peak-picking tuners, delivering reliable pitch tracking down to the low E2 string (82.41 Hz).

## Tech Stack
- **Microcontroller:** ESP32-S3 DevKitC
- **Audio Sensor:** INMP441 Omnidirectional I2S MEMS Microphone
- **Display:** 0.96" 128x64 Dual-Color (Yellow/Blue) I2C OLED (SSD1306)
- **Framework:** Arduino Core for ESP32 / PlatformIO (utilizing native ESP-IDF `driver/i2s.h`)
- **Libraries:** `kosme/arduinoFFT`, `adafruit/Adafruit GFX Library`, `adafruit/Adafruit SSD1306`
- **DSP Pipeline:** 8kHz Sample Rate, 2048-sample FFT (~3.91 Hz raw bin width), 3rd-order Harmonic Product Spectrum (HPS), Parabolic Interpolation

## Hardware Wiring

### INMP441 I2S Microphone
The `L/R` pin is tied to ground so the microphone transmits exclusively on the Left Channel of the I2S frame.

| INMP441 Pin | ESP32-S3 Pin | Purpose |
| :--- | :--- | :--- |
| **VDD** | 3.3V | Power Supply |
| **GND** | GND | Common Ground |
| **SCK** | GPIO 4 | Serial Clock (BCLK) |
| **WS** | GPIO 5 | Word Select (LRCLK) |
| **SD** | GPIO 6 | Serial Data |
| **L/R** | GND | Channel Selection (Left Channel) |

### SSD1306 I2C OLED Display
| OLED Pin | ESP32-S3 Pin | Purpose |
| :--- | :--- | :--- |
| **VCC** | 3.3V | Power Supply |
| **GND** | GND | Common Ground |
| **SDA** | GPIO 8 | I2C Data Line |
| **SCL** | GPIO 9 | I2C Clock Line |

## Project Architecture
```text
├── include/
│   ├── audio_i2s.h      # I2S hardware pinout, DMA constants, and driver interface
│   ├── tuner_math.h     # DSP parameters, NoteResult struct, and pitch engine interface
│   └── display_ui.h     # OLED I2C pins and UI rendering declarations
├── src/
│   ├── audio_i2s.cpp    # Native ESP-IDF I2S & DMA ring buffer acquisition
│   ├── tuner_math.cpp   # DC removal, Hamming window, FFT, HPS, parabolic fit, 12-TET mapping
│   ├── display_ui.cpp   # Dual-zone OLED tuning bar and note telemetry rendering
│   └── main.cpp         # System initialization and real-time Input -> Process -> Output loop
└── platformio.ini       # Board configuration and library dependencies
```

## Key Engineering Decisions
- **Native ESP-IDF I2S & Blocking DMA Sync:** Configured the hardware I2S controller with `I2S_CHANNEL_FMT_ONLY_LEFT` and DMA ring buffers. Using blocking DMA reads (`portMAX_DELAY`) synchronizes the main loop (~3.9 Hz UI refresh rate at 2048 samples / 8kHz) directly to the hardware sample clock without busy-waiting or dropping audio frames.
- **Harmonic Product Spectrum (HPS) for Octave Stability:** Acoustic guitar fundamentals—especially low E2 (82.41 Hz) and A2 (110.00 Hz)—often carry stronger 2nd or 3rd harmonics than the fundamental frequency. Multiplying downsampled spectral bins (`vReal[i] * vReal[2*i] * vReal[3*i]`) across the 65 Hz–1000 Hz guitar band reinforces the true fundamental and prevents octave-jump errors.
- **Sub-Bin Precision via Parabolic Interpolation:** With a 2048-point FFT at 8kHz, raw bin spacing is ~3.906 Hz. After refining the HPS peak against adjacent magnitude bins, a 3-point parabolic curve fit estimates the fractional bin offset, resolving frequencies to `< 0.1 Hz` accuracy for precise cent calculation.
- **Static Memory & Zero-Allocation DSP:** All working buffers (`rawSamples`, `vReal`, `vImag`, `hpsSpectrum`) are statically allocated in global/BSS memory rather than the task stack or dynamic heap, preventing stack overflows and long-term heap fragmentation.
- **Dual-Zone OLED Ergonomics:** Mapped the UI to the physical color split of the 128x64 panel: the top 16px yellow zone renders a `±50 cent` needle with a solid center lock block within `±3 cents`, while the bottom 48px blue zone displays the note name, octave, measured Hz, cent deviation, and target frequency.

## Acceptance Criteria & Testing
- **Pitch & Octave Accuracy:** Reliably locks onto standard open strings (`E2 - A2 - D3 - G3 - B3 - E4`) and fretted notes without jumping to upper harmonics on low strings.
- **Noise Floor Rejection:** Ambient noise below `NOISE_THRESHOLD` is gated before FFT execution, keeping the UI in an idle `"Tel bekleniyor"` state without erratic needle jitter.
- **Visual Lock Feedback:** Smooth needle movement across `±50 cents` with clear `"TAM OK!"` lock feedback within `±3 cents` of the 12-TET (`A4 = 440 Hz`) reference.

## Author
**Tolga Han Kaya**  
[LinkedIn](https://www.linkedin.com/in/tolga-han-kaya-ccba%C2%AE-439a74136/) | [GitHub](https://github.com/TolgaHanKaya)

## Acknowledgments
- **AI Assistance:** Code scaffolding, syntax formatting, and documentation structuring were accelerated using AI tools, while architectural decisions, hardware integration, and QA testing were manually executed.