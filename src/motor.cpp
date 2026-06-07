#include "motor.h"

extern Mode currentMode;
extern Mode modeAfterCalibration;
extern bool calibrationComboLockout;
extern unsigned long calibrationStartTime;
extern String statusMessage;

static MotorDir currentDir = STOP;
static unsigned long motorStopTime = 0;

// ─── Управление мотором ───────────────
// Остановка мотора
void motorStop() {
  ledcWrite(PWM_CHANNEL, 0);
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  motorStopTime = 0;
  currentDir = STOP;
}

// Мотор Влево
void motorLeft() {
  digitalWrite(MOTOR_IN1, HIGH);
  digitalWrite(MOTOR_IN2, LOW);
  ledcWrite(PWM_CHANNEL, MOTOR_SPEED);
  currentDir = LEFT;
}

// Мотор Вправо
void motorRight() {
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, HIGH);
  ledcWrite(PWM_CHANNEL, MOTOR_SPEED);
  currentDir = RIGHT;
}

// Обновление состояния мотора (остановка по таймеру)
void updateMotor(unsigned long now) {
  if (motorStopTime != 0 && now >= motorStopTime) {
    motorStop();
  }
}

// Установка времени остановки мотора
void setMotorStopTime(unsigned long stopTime) {
  motorStopTime = stopTime;
}

// Получение текущего направления мотора
MotorDir getMotorDir() {
  return currentDir;
}

// Автоматическое позиционирование (поворот) 
//двигателя в зависимости от разности напряжений на датчиках
void autoTrack(unsigned long now, float vLeft, float vRight) {
  float diff = vLeft - vRight;

  if (diff > VOLTAGE_THRESHOLD_MV) {
    motorLeft();
    setMotorStopTime(now + MOTOR_RUN_MS);
  } else if (-diff > VOLTAGE_THRESHOLD_MV) {
    motorRight();
    setMotorStopTime(now + MOTOR_RUN_MS);
  } else {
    motorStop();
  }
}

// Ручной поворот в зависимости от нажатых кнопок
void handleManual() {
  static unsigned long bothPressStart = 0;
  bool btnL = (digitalRead(BTN_LEFT) == LOW);
  bool btnR = (digitalRead(BTN_RIGHT) == LOW);

  if (calibrationComboLockout) {
    if (!btnL && !btnR) {
      calibrationComboLockout = false;
    } else {
      motorStop();
      return;
    }
  }

  if (btnL && btnR) {
    motorStop();
    setMotorStopTime(0);

    if (bothPressStart == 0) {
      bothPressStart = millis();
    }

    if (millis() - bothPressStart >= CALIBRATION_HOLD_MS) {
      modeAfterCalibration = MANUAL;
      currentMode = CALIBRATION;
      calibrationStartTime = millis();
      calibrationComboLockout = true;
      statusMessage = "Режим калибровки активирован";
      bothPressStart = 0;
      Serial.println("\n=== РЕЖИМ КАЛИБРОВКИ ===");
      Serial.println("Обе панели должны быть под одинаковым освещением.");
      Serial.println("Снимаю оба канала одновременно и подгоняю CAL L = CAL R.");
    }
  } else if (btnL && !btnR) {
    bothPressStart = 0;
    motorLeft();
    setMotorStopTime(0);
  } else if (btnR && !btnL) {
    bothPressStart = 0;
    motorRight();
    setMotorStopTime(0);
  } else {
    bothPressStart = 0;
    motorStop();
  }
}
