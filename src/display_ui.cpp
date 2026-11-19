#include "display_ui.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

constexpr int SCREEN_WIDTH  = 128;
constexpr int SCREEN_HEIGHT = 64;
constexpr int YELLOW_ZONE_H = 16; // Top yellow zone height in pixels

static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void initDisplayUI() {
    Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);

    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
        Serial.println("SSD1306 OLED initialization failed!");
        for (;;);
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    // Boot splash screen
    display.setTextSize(1);
    display.setCursor(22, 4);
    display.print("ESP32-S3 TUNER");
    display.setCursor(28, 34);
    display.print("Listening...");
    display.display();
}

static void drawTuningBar(float cents, bool valid) {
    const int centerX = SCREEN_WIDTH / 2;

    // Outer -50 / +50 cent boundary markers and center target notches
    display.drawFastVLine(4, 3, 10, SSD1306_WHITE);
    display.drawFastVLine(SCREEN_WIDTH - 5, 3, 10, SSD1306_WHITE);
    display.drawFastVLine(centerX, 0, 4, SSD1306_WHITE);
    display.drawFastVLine(centerX, 12, 4, SSD1306_WHITE);

    if (!valid) {
        display.drawPixel(centerX, 8, SSD1306_WHITE);
        return;
    }

    // Map cent offset (-50 to +50) to horizontal pixel coordinates
    float clampedCents = constrain(cents, -50.0f, 50.0f);
    int needleX = centerX + static_cast<int>((clampedCents / 50.0f) * 56.0f);

    // Render solid lock block within ±3 cents, otherwise draw moving needle
    if (abs(cents) <= 3.0f) {
        display.fillRect(centerX - 6, 3, 13, 10, SSD1306_WHITE);
    } else {
        display.fillRect(needleX - 2, 2, 5, 12, SSD1306_WHITE);
    }
}

void updateDisplayUI(const NoteResult& note) {
    display.clearDisplay();

    // Top yellow zone (y: 0..15): Tuning bar
    drawTuningBar(note.cents, note.valid);

    // Bottom blue zone (y: 16..63): Note and frequency telemetry
    if (!note.valid) {
        display.setTextSize(2);
        display.setCursor(46, 30);
        display.print("--");

        display.setTextSize(1);
        display.setCursor(22, 52);
        display.print("Pluck a string");
        display.display();
        return;
    }

    // Note name and SPN octave
    display.setTextSize(4);
    display.setCursor(8, 22);
    display.print(note.name);

    int octaveX = (strlen(note.name) > 1) ? 58 : 36;
    display.setTextSize(2);
    display.setCursor(octaveX, 36);
    display.print(note.octave);

    // Measured frequency, cent offset, and target frequency
    display.setTextSize(1);
    display.setCursor(74, 24);
    display.printf("%.1fHz", note.frequency);

    display.setCursor(74, 38);
    if (abs(note.cents) <= 3.0f) {
        display.print("IN TUNE!");
    } else {
        display.printf("%+.0f cent", note.cents);
    }

    display.setCursor(74, 52);
    display.printf("->%.1f", note.targetFreq);

    display.display();
}