#pragma once

#include <Arduino.h>
#include <driver/i2s.h>

// ESP32-S3 I2S GPIO pins (INMP441 L/R tied to GND -> Left Channel)
constexpr int I2S_SCK_PIN = 4;  // Serial Clock (BCLK)
constexpr int I2S_WS_PIN  = 5;  // Word Select (LRCLK)
constexpr int I2S_SD_PIN  = 6;  // Serial Data (DOUT)

// Audio and DMA buffer settings
constexpr i2s_port_t I2S_PORT    = I2S_NUM_0;
constexpr uint32_t   SAMPLE_RATE = 8000;  // 8 kHz sample rate (Nyquist: 4 kHz)
constexpr size_t     BUFFER_SIZE = 2048;  // Samples per FFT window (256 ms)

void initAudioI2S();
size_t readAudioSamples(int32_t* buffer, size_t numSamples);