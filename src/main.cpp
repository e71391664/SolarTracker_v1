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
#include <INA226_WE.h>
#include <EEPROM.h>
#include "display.h"

// ─── INA226 ────────────────────────────────────────────────
#define INA_LEFT_ADDR  0x40   // левая панель
#define INA_RIGHT_ADDR 0x41   // правая панель
INA226_WE inaLeft(INA_LEFT_ADDR);
INA226_WE inaRight(INA_RIGHT_ADDR);

// ─── L298N пины ────────────────────────────────────────────
#define MOTOR_IN1  4
#define MOTOR_IN2  5
#define MOTOR_ENA  6

// ─── Кнопки ────────────────────────────────────────────────
#define BTN_AUTO   7
#define BTN_LEFT   2
#define BTN_RIGHT  3

// ─── Настройки трекера ─────────────────────────────────────
#define VOLTAGE_THRESHOLD_MV  150.0f  // мВ — мёртвая зона
#define MOTOR_SPEED           180    // PWM 0-255
#define MOTOR_RUN_MS          600    // мс импульс поворота
#define MEASURE_INTERVAL_MS   500    // интервал замеров
#define CALIBRATION_HOLD_MS   1000   // удержание LEFT+RIGHT в MANUAL для калибровки

// ─── PWM (ESP32 Arduino Core v3.x) ────────────────────────
#define PWM_FREQ      5000
#define PWM_RESOLUTION 8
// PWM канал для ESP32 (0-15)
#define PWM_CHANNEL    0

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
MotorDir  lastDir     = STOP;
float     vLeft       = 0;
float     vRight      = 0;
float     vLeftRaw    = 0;
float     vRightRaw   = 0;
unsigned long lastMeasure = 0;
unsigned long motorStopTime = 0;
unsigned long calibrationStartTime = 0;
bool calibrationComboLockout = false;

// ─── Прототипы ─────────────────────────────────────────────
void motorStop();
void motorLeft();
void motorRight();
void readVoltages();
void autoTrack(unsigned long now);
void handleManual();
void updateMotor(unsigned long now);
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
  if (!inaLeft.init()) {
    Serial.println("INA226 LEFT (0x40) не найден!");
  }
  if (!inaRight.init()) {
    Serial.println("INA226 RIGHT (0x41) не найден!");
  }
  // Шунт 0.1 Ом, макс ток 3.2А (для малых панелей достаточно)
  inaLeft.setResistorRange(0.1f, 3.2f);
  inaRight.setResistorRange(0.1f, 3.2f);
  inaLeft.setCorrectionFactor(1.0f);
  inaRight.setCorrectionFactor(1.0f);

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
      autoTrack(now);
    } else if (currentMode == CALIBRATION) {
      handleCalibration();
    }
    updateOLED(lastDir);
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
  vLeftRaw  = inaLeft.getBusVoltage_V()  * 1000.0f;
  vRightRaw = inaRight.getBusVoltage_V() * 1000.0f;
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

// ══════════════════════════════════════════════════════════
// Автоматический дифференциальный трекинг
void autoTrack(unsigned long now) {
  float diff = vLeft - vRight;

  if (diff > VOLTAGE_THRESHOLD_MV) {
    // Левая панель освещена сильнее → трекер поворачивает влево
    motorLeft();
    lastDir = LEFT;
    motorStopTime = now + MOTOR_RUN_MS;
  } else if (-diff > VOLTAGE_THRESHOLD_MV) {
    // Правая панель освещена сильнее → поворот вправо
    motorRight();
    lastDir = RIGHT;
    motorStopTime = now + MOTOR_RUN_MS;
  } else {
    motorStop();
    lastDir = STOP;
  }
}

void updateMotor(unsigned long now) {
  if (motorStopTime != 0 && now >= motorStopTime) {
    motorStop();
    lastDir = STOP;
  }
}

// ══════════════════════════════════════════════════════════
// Ручное управление кнопками
void handleManual() {
  static unsigned long bothPressStart = 0;
  bool btnL = (digitalRead(BTN_LEFT)  == LOW);
  bool btnR = (digitalRead(BTN_RIGHT) == LOW);

  if (calibrationComboLockout) {
    if (!btnL && !btnR) {
      calibrationComboLockout = false;
    } else {
      motorStop();
      lastDir = STOP;
      return;
    }
  }

  if (btnL && btnR) {
    motorStop();
    lastDir = STOP;
    motorStopTime = 0;

    if (bothPressStart == 0) {
      bothPressStart = millis();
    }

    if (millis() - bothPressStart >= CALIBRATION_HOLD_MS) {
      modeAfterCalibration = MANUAL;
      currentMode = CALIBRATION;
      calibrationStartTime = millis();
      calibrationComboLockout = true;
      bothPressStart = 0;
      Serial.println("\n=== РЕЖИМ КАЛИБРОВКИ ===");
      Serial.println("Обе панели должны быть под одинаковым освещением.");
      Serial.println("Снимаю оба канала одновременно и подгоняю CAL L = CAL R.");
    }
  } else if (btnL && !btnR) {
    bothPressStart = 0;
    motorLeft();
    lastDir = LEFT;
    motorStopTime = 0;
  } else if (btnR && !btnL) {
    bothPressStart = 0;
    motorRight();
    lastDir = RIGHT;
    motorStopTime = 0;
  } else {
    bothPressStart = 0;
    motorStop();
    lastDir = STOP;
  }
}

// ══════════════════════════════════════════════════════════
// Управление мотором
void motorStop() {
  ledcWrite(PWM_CHANNEL, 0);
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  motorStopTime = 0;
}

void motorLeft() {
  digitalWrite(MOTOR_IN1, HIGH);
  digitalWrite(MOTOR_IN2, LOW);
  ledcWrite(PWM_CHANNEL, MOTOR_SPEED);
}

void motorRight() {
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, HIGH);
  ledcWrite(PWM_CHANNEL, MOTOR_SPEED);
}


