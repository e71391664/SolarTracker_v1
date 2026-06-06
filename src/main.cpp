/*
 * ============================================================
 *  SOLAR TRACKER — ESP32-C3 Super Mini
 * ============================================================
 *  Железо:
 *   - 2x солнечная панель АК 68x36
 *   - 2x INA226  (замер напряжения панелей)
 *       INA226 #1 (левая панель)  I2C addr 0x40
 *       INA226 #2 (правая панель) I2C addr 0x41
 *   - OLED 128x64 SSD1306         I2C addr 0x3C
 *   - L298N  → DC мотор (ось восток/запад)
 *   - ESP32-C3 Super Mini
 *
 *  Алгоритм: дифференциальный трекинг
 *   Если V_left > V_right + THRESHOLD → поворот ВЛЕВО (на запад)
 *   Если V_right > V_left + THRESHOLD → поворот ВПРАВО (на восток)
 *   Иначе → стоп (солнце найдено)
 *
 *  Пины (ESP32-C3 Super Mini):
 *   I2C SDA  → GPIO 8
 *   I2C SCL  → GPIO 9
 *   L298N IN1 → GPIO 4   (направление А)
 *   L298N IN2 → GPIO 5   (направление Б)
 *   L298N ENA → GPIO 6   (PWM скорость)
 *   Кнопка AUTO/MANUAL → GPIO 7 (подтяжка INPUT_PULLUP)
 *   Кнопка ВЛЕВО  → GPIO 2
 *   Кнопка ВПРАВО → GPIO 3
 * ============================================================
 *
 *  Библиотеки (установить через Library Manager):
 *   - Adafruit GFX Library
 *   - Adafruit SSD1306
 *   - INA226_WE  (by Wolfgang Ewald)
 *   - Wire (встроенная)
 * ============================================================
 */

#include <Wire.h>
#include <EEPROM.h>
#include "display.h"
#include "motor.h"
#include "ina226.h"

// ─── Настройки трекера ─────────────────────────────────────
#define MEASURE_INTERVAL_MS   500    // интервал замеров

// ─── Структура калибровки ──────────────────────────────────
struct CalibrationData {
  float coeff_left;   // коэффициент коррекции левой панели
  float coeff_right;  // коэффициент коррекции правой панели
  uint32_t magic;     // магическое число для проверки валидности (0xCAL1B0A)
};

// EEPROM адреса
#define EEPROM_CALIB_ADDR 0
#define EEPROM_SIZE       512
#define CALIB_MAGIC       0xCA11B0A

CalibrationData calibration = { 1.0f, 1.0f, CALIB_MAGIC };

Mode      currentMode = AUTO;
Mode      modeAfterCalibration = MANUAL;
float     vLeft       = 0;
float     vRight      = 0;
float     vLeftRaw    = 0;
float     vRightRaw   = 0;
unsigned long lastMeasure = 0;
unsigned long calibrationStartTime = 0;
bool calibrationComboLockout = false;

// ─── Прототипы ─────────────────────────────────────────────
void readVoltages();
void loadCalibration();
void saveCalibration();
void applyCalibration();
void resetCalibration();
void handleCalibration();

// ══════════════════════════════════════════════════════════
// Работа с EEPROM (калибровка)
void loadCalibration() {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(EEPROM_CALIB_ADDR, calibration);
  EEPROM.end();
  
  // Проверка валидности
  if (calibration.magic != CALIB_MAGIC) {
    resetCalibration();
    Serial.println("⚠ Калибровка не найдена, используются коэффициенты по умолчанию");
  } else {
    Serial.printf("✓ Калибровка загружена: L=%.4f, R=%.4f\n", 
                  calibration.coeff_left, calibration.coeff_right);
  }
}

void saveCalibration() {
  calibration.magic = CALIB_MAGIC;
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.put(EEPROM_CALIB_ADDR, calibration);
  EEPROM.commit();
  EEPROM.end();
  Serial.printf("✓ Калибровка сохранена: L=%.4f, R=%.4f\n", 
                calibration.coeff_left, calibration.coeff_right);
}

void resetCalibration() {
  calibration.coeff_left = 1.0f;
  calibration.coeff_right = 1.0f;
  calibration.magic = CALIB_MAGIC;
  saveCalibration();
  Serial.println("✓ Калибровка сброшена на значения по умолчанию");
}

void applyCalibration() {
  vLeft  *= calibration.coeff_left;
  vRight *= calibration.coeff_right;
}

// ══════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);

  // I2C (SDA=8, SCL=9 для C3 Super Mini)
  Wire.begin(8, 9);

  // OLED
  initDisplay();

  // INA226
  initINA226();

  // L298N
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(MOTOR_ENA, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, 0);
  motorStop();

  // Кнопки
  pinMode(BTN_AUTO,  INPUT_PULLUP);
  pinMode(BTN_LEFT,  INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);

  // Загрузка калибровки из EEPROM
  loadCalibration();

  Serial.println("Инициализация завершена");
}

// ══════════════════════════════════════════════════════════
void loop() {
  // Переключение режима AUTO/MANUAL
  if (digitalRead(BTN_AUTO) == LOW) {
    delay(50);
    if (digitalRead(BTN_AUTO) == LOW) {
      currentMode = (currentMode == AUTO) ? MANUAL : AUTO;
      motorStop();
      delay(300);
    }
  }

  // Замер раз в MEASURE_INTERVAL_MS
  unsigned long now = millis();
  if (now - lastMeasure >= MEASURE_INTERVAL_MS) {
    lastMeasure = now;
    readVoltages();

    if (currentMode == AUTO) {
      autoTrack(now, vLeft, vRight);
    } else if (currentMode == CALIBRATION) {
      handleCalibration();
    }
    updateOLED(getMotorDir());
  }

  // Ручное управление (опрашиваем постоянно)
  if (currentMode == MANUAL) {
    handleManual();
  }

  updateMotor(now);
}

// ══════════════════════════════════════════════════════════
// Чтение напряжений с обеих панелей (мВ)
void readVoltages() {
  vLeftRaw  = getLeftVoltage_mV();
  vRightRaw = getRightVoltage_mV();
  vLeft     = vLeftRaw;
  vRight    = vRightRaw;

  // Применение калибровки
  applyCalibration();

  Serial.printf("RAW: L=%.1f mV  R=%.1f mV  diff=%.1f mV\n",
                vLeftRaw, vRightRaw, vLeftRaw - vRightRaw);
  Serial.printf("CAL: L=%.1f mV  R=%.1f mV  diff=%.1f mV  coeff L=%.4f R=%.4f\n",
                vLeft, vRight, vLeft - vRight,
                calibration.coeff_left, calibration.coeff_right);
}

// ══════════════════════════════════════════════════════════
// Режим калибровки
void handleCalibration() {
  if (vLeftRaw <= 1.0f || vRightRaw <= 1.0f) {
    Serial.println("Калибровка отменена: слишком малое напряжение на одном из каналов.");
    currentMode = modeAfterCalibration;
    return;
  }

  float target = (vLeftRaw > vRightRaw) ? vLeftRaw : vRightRaw;

  calibration.coeff_left = target / vLeftRaw;
  calibration.coeff_right = target / vRightRaw;

  vLeft = vLeftRaw;
  vRight = vRightRaw;
  applyCalibration();

  Serial.printf("\n=== КАЛИБРОВКА ЗАВЕРШЕНА ===\n");
  Serial.printf("RAW: L=%.1f mV  R=%.1f mV  diff=%.1f mV\n",
                vLeftRaw, vRightRaw, vLeftRaw - vRightRaw);
  Serial.printf("CAL: L=%.1f mV  R=%.1f mV  diff=%.1f mV\n",
                vLeft, vRight, vLeft - vRight);
  Serial.printf("Коэффициенты: L=%.4f  R=%.4f\n",
                calibration.coeff_left, calibration.coeff_right);

  saveCalibration();

  currentMode = modeAfterCalibration;
  Serial.println(currentMode == MANUAL ? "Переход в режим MANUAL" : "Переход в режим AUTO");
}