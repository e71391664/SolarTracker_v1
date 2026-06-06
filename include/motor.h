#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>
#include "display.h"

// ─── L298N пины ────────────────────────────────────────────────
#define MOTOR_IN1  4
#define MOTOR_IN2  5
#define MOTOR_ENA  6

// ─── Кнопки управления мотором ─────────────────────────────────
#define BTN_AUTO   7
#define BTN_LEFT   2
#define BTN_RIGHT  3

// ─── Настройки двигателя ──────────────────────────────────────
#define MOTOR_SPEED  180

// ─── Управление трекингом ────────────────────────────────────
#define VOLTAGE_THRESHOLD_MV  150.0f  // мВ — мёртвая зона
#define MOTOR_RUN_MS          600    // мс импульс поворота
#define CALIBRATION_HOLD_MS   1000   // удержание LEFT+RIGHT в MANUAL для калибровки

// ─── PWM (ESP32 Arduino Core v3.x) ────────────────────────
#define PWM_FREQ       5000
#define PWM_RESOLUTION 8
#define PWM_CHANNEL    0

void motorStop();
void motorLeft();
void motorRight();
void updateMotor(unsigned long now);
void setMotorStopTime(unsigned long stopTime);
MotorDir getMotorDir();
void autoTrack(unsigned long now, float vLeft, float vRight);
void handleManual();

#endif // MOTOR_H
