#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>

// ─── L298N пины ────────────────────────────────────────────────
#define MOTOR_IN1  4
#define MOTOR_IN2  5
#define MOTOR_ENA  6

// ─── Настройки двигателя ──────────────────────────────────────
#define MOTOR_SPEED  180

// ─── PWM (ESP32 Arduino Core v3.x) ────────────────────────
#define PWM_FREQ       5000
#define PWM_RESOLUTION 8
#define PWM_CHANNEL    0

void motorStop();
void motorLeft();
void motorRight();

#endif // MOTOR_H
