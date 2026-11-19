#include <Arduino.h>
#include "audio_i2s.h"
#include "tuner_math.h"
#include "display_ui.h"

// Static buffer for raw 32-bit I2S audio frames
int32_t rawSamples[BUFFER_SIZE];

void setup() {
    Serial.begin(115200);

    // Initialize OLED display, I2S microphone driver, and DSP engine
    initDisplayUI();
    initAudioI2S();
    initTunerMath();
}

void loop() {
    // Capture audio block via blocking DMA read, run pitch detection, and refresh UI
    size_t samplesRead = readAudioSamples(rawSamples, BUFFER_SIZE);

    if (samplesRead == BUFFER_SIZE) {
        NoteResult note = analyzeAudio(rawSamples, samplesRead);
        updateDisplayUI(note);
    }
}