#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ─── OLED ──────────────────────────────────────────────────
#define SCREEN_W   128
#define SCREEN_H    64
#define OLED_ADDR  0x3C

extern Adafruit_SSD1306 display;

// ─── Состояние ─────────────────────────────────────────────
enum MotorDir { STOP, LEFT, RIGHT };
enum Mode     { AUTO, MANUAL, CALIBRATION };

void initDisplay();
void updateOLED(MotorDir dir);

#endif
