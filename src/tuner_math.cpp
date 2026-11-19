#include "tuner_math.h"
#include "audio_i2s.h"
#include <arduinoFFT.h>
#include <math.h>

// Static DSP working buffers to avoid dynamic heap allocation
static double vReal[BUFFER_SIZE];
static double vImag[BUFFER_SIZE];
static double hpsSpectrum[BUFFER_SIZE / 3];

static ArduinoFFT<double> FFT(vReal, vImag, BUFFER_SIZE, SAMPLE_RATE);

static const char* NOTE_NAMES[12] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

constexpr int32_t NOISE_THRESHOLD = 1000;

void initTunerMath() {
}

// Maps a fundamental frequency (Hz) to the nearest 12-TET note and cent deviation
static NoteResult frequencyToNote(float freq) {
    NoteResult res;
    res.valid = true;
    res.frequency = freq;

    float semitonesFromA4 = 12.0f * log2f(freq / 440.0f);
    int nearestSemitone = static_cast<int>(roundf(semitonesFromA4));

    res.targetFreq = 440.0f * powf(2.0f, nearestSemitone / 12.0f);
    res.cents = (semitonesFromA4 - nearestSemitone) * 100.0f;

    int midiNote = 69 + nearestSemitone;
    int noteIndex = ((midiNote % 12) + 12) % 12;
    res.octave = (midiNote / 12) - 1;
    res.name = NOTE_NAMES[noteIndex];

    return res;
}

NoteResult analyzeAudio(const int32_t* rawSamples, size_t sampleCount) {
    int32_t maxAmp = 0;

    // Normalize 24-bit left-aligned I2S frames to 18-bit signed values and track peak amplitude
    for (size_t i = 0; i < sampleCount; i++) {
        int32_t val = rawSamples[i] >> 14;
        if (abs(val) > maxAmp) {
            maxAmp = abs(val);
        }
        vReal[i] = static_cast<double>(val);
        vImag[i] = 0.0;
    }

    if (maxAmp < NOISE_THRESHOLD) {
        return {false, "-", 0, 0.0f, 0.0f, 0.0f};
    }

    // Compute FFT magnitude spectrum
    FFT.dcRemoval();
    FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);
    FFT.compute(FFTDirection::Forward);
    FFT.complexToMagnitude();

    // Guitar fundamental search band: ~65 Hz (bin 16) to ~1000 Hz (bin 256)
    const size_t minBin = static_cast<size_t>((65.0 * BUFFER_SIZE) / SAMPLE_RATE);
    const size_t maxBin = static_cast<size_t>((1000.0 * BUFFER_SIZE) / SAMPLE_RATE);

    // Step 1: 3rd-order Harmonic Product Spectrum (1x * 2x * 3x) to suppress octave jumps
    double maxHpsVal = 0.0;
    size_t bestBin = 0;

    for (size_t i = minBin; i < maxBin; i++) {
        double hpsVal = vReal[i] * vReal[2 * i] * vReal[3 * i];
        hpsSpectrum[i] = hpsVal;

        if (hpsVal > maxHpsVal) {
            maxHpsVal = hpsVal;
            bestBin = i;
        }
    }

    if (bestBin < minBin || maxHpsVal <= 0.0) {
        return {false, "-", 0, 0.0f, 0.0f, 0.0f};
    }

    // Step 2: Refine peak bin against immediate neighbors in the raw magnitude spectrum
    size_t peakBin = bestBin;
    if (bestBin > 1 && bestBin < (BUFFER_SIZE / 2) - 1) {
        if (vReal[bestBin - 1] > vReal[peakBin]) peakBin = bestBin - 1;
        if (vReal[bestBin + 1] > vReal[peakBin]) peakBin = bestBin + 1;
    }

    // Step 3: 3-point parabolic interpolation for sub-bin frequency resolution
    double alpha = vReal[peakBin - 1];
    double beta  = vReal[peakBin];
    double gamma = vReal[peakBin + 1];
    double denominator = alpha - 2.0 * beta + gamma;
    double delta = (denominator != 0.0) ? 0.5 * (alpha - gamma) / denominator : 0.0;

    double exactFreq = (static_cast<double>(peakBin) + delta) * (static_cast<double>(SAMPLE_RATE) / BUFFER_SIZE);

    if (exactFreq < 65.0 || exactFreq > 1100.0) {
        return {false, "-", 0, 0.0f, 0.0f, 0.0f};
    }

    return frequencyToNote(static_cast<float>(exactFreq));
}