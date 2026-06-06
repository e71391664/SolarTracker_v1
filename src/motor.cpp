#include "motor.h"

extern unsigned long motorStopTime;

// ─── Управление мотором ───────────────
// Остановка мотора
void motorStop() {
  ledcWrite(PWM_CHANNEL, 0);
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  motorStopTime = 0;
}

//Мотор Влево
void motorLeft() {
  digitalWrite(MOTOR_IN1, HIGH);
  digitalWrite(MOTOR_IN2, LOW);
  ledcWrite(PWM_CHANNEL, MOTOR_SPEED);
}

//Мотор Вправо
void motorRight() {
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, HIGH);
  ledcWrite(PWM_CHANNEL, MOTOR_SPEED);
}
