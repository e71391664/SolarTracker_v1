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
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <INA226_WE.h>

// ─── OLED ──────────────────────────────────────────────────
#define SCREEN_W   128
#define SCREEN_H    64
#define OLED_ADDR  0x3C
Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, -1);

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
#define VOLTAGE_THRESHOLD_MV  50.0f  // мВ — мёртвая зона
#define MOTOR_SPEED           180    // PWM 0-255
#define MOTOR_RUN_MS          300    // мс импульс поворота
#define MEASURE_INTERVAL_MS   500    // интервал замеров

// ─── PWM (ESP32 Arduino Core v3.x) ────────────────────────
#define PWM_FREQ      5000
#define PWM_RESOLUTION 8
// PWM канал для ESP32 (0-15)
#define PWM_CHANNEL    0

// ─── Состояние ─────────────────────────────────────────────
enum MotorDir { STOP, LEFT, RIGHT };
enum Mode     { AUTO, MANUAL };

Mode      currentMode = AUTO;
MotorDir  lastDir     = STOP;
float     vLeft       = 0;
float     vRight      = 0;
unsigned long lastMeasure = 0;
unsigned long motorStopTime = 0;

// ─── Прототипы ─────────────────────────────────────────────
void motorStop();
void motorLeft();
void motorRight();
void updateOLED(MotorDir dir);
void readVoltages();
void autoTrack(unsigned long now);
void handleManual();
void updateMotor(unsigned long now);

// ══════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);

  // I2C (SDA=8, SCL=9 для C3 Super Mini)
  Wire.begin(8, 9);

  // OLED
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
  vLeft  = inaLeft.getBusVoltage_V()  * 1000.0f;
  vRight = inaRight.getBusVoltage_V() * 1000.0f;

   // Добавь:
  Serial.printf("INA_L raw bus: %.4f V\n", inaLeft.getBusVoltage_V());
  Serial.printf("INA_R raw bus: %.4f V\n", inaRight.getBusVoltage_V());
  Serial.printf("V_left=%.1f mV  V_right=%.1f mV  diff=%.1f mV\n",
                vLeft, vRight, vLeft - vRight);          
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
  bool btnL = (digitalRead(BTN_LEFT)  == LOW);
  bool btnR = (digitalRead(BTN_RIGHT) == LOW);

  if (btnL && !btnR) {
    motorLeft();
    lastDir = LEFT;
    motorStopTime = 0;
  } else if (btnR && !btnL) {
    motorRight();
    lastDir = RIGHT;
    motorStopTime = 0;
  } else {
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

// ══════════════════════════════════════════════════════════
// Обновление OLED
void updateOLED(MotorDir dir) {
  display.clearDisplay();

  // ── Заголовок ──
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("SOLAR TRACKER");
  display.setCursor(90, 0);
  display.print(currentMode == AUTO ? "AUTO" : "MAN");

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