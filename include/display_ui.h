#pragma once

#include <Arduino.h>
#include "tuner_math.h"

// ESP32-S3 I2C pins and OLED configuration
constexpr int OLED_SDA_PIN = 8;
constexpr int OLED_SCL_PIN = 9;
constexpr uint8_t OLED_I2C_ADDR = 0x3C;

void initDisplayUI();
void updateDisplayUI(const NoteResult& note);