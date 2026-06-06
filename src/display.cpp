#include "display.h"
#include <Wire.h>

// ─── Глобальная переменная дисплея ─────────────────────────
Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, -1);

// ─── Внешние переменные из main.cpp ────────────────────────
extern float vLeft;
extern float vRight;
extern Mode currentMode;

// ══════════════════════════════════════════════════════════
// Инициализация OLED
void initDisplay() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("SSD1306 не найден!");
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 24);
  display.setTextSize(1);
  display.println("  Solar Tracker v1.0");
  display.display();
  delay(1500);
}

// ══════════════════════════════════════════════════════════
// Обновление OLED
void updateOLED(MotorDir dir) {
  display.clearDisplay();

  // ── Заголовок ──
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("SOLAR TRACKER");
  display.setCursor(90, 0);
  if (currentMode == AUTO) {
    display.print("AUTO");
  } else if (currentMode == MANUAL) {
    display.print("MAN");
  } else {
    display.print("CAL");
  }

  // ── Разделитель ──
  display.drawLine(0, 9, 127, 9, SSD1306_WHITE);

  // ── Напряжения ──
  display.setCursor(0, 13);
  display.printf("L: %6.0f mV", vLeft);
  display.setCursor(0, 24);
  display.printf("R: %6.0f mV", vRight);

  // ── Разница ──
  float diff = vLeft - vRight;
  display.setCursor(0, 35);
  display.printf("dV:%+6.0f mV", diff);

  // ── Разделитель ──
  display.drawLine(0, 45, 127, 45, SSD1306_WHITE);

  // ── Статус мотора ──
  display.setTextSize(1);
  display.setCursor(0, 49);
  switch (dir) {
    case LEFT:
      display.print("<<< LEFT <<<");
      // Стрелка влево
     // display.fillTriangle(110, 56, 127, 49, 127, 63, SSD1306_WHITE);
      break;
    case RIGHT:
      display.print(">>> RIGHT >>>");
      //display.fillTriangle(18, 56, 1, 49, 1, 63, SSD1306_WHITE);
      break;
    case STOP:
    default:
      display.print("=== STOP ===");
      break;
  }

  display.display();
}
