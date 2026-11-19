#pragma once

#include <Arduino.h>

// Pitch detection result passed to the display module
struct NoteResult {
    bool valid;
    const char* name;
    int octave;
    float frequency;
    float targetFreq;
    float cents;
};

void initTunerMath();
NoteResult analyzeAudio(const int32_t* rawSamples, size_t sampleCount);