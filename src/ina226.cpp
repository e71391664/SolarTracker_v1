#include "ina226.h"
#include <Arduino.h>

static INA226_WE inaLeft(INA_LEFT_ADDR);
static INA226_WE inaRight(INA_RIGHT_ADDR);

void initINA226() {
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
}

float getLeftVoltage_mV() {
  return inaLeft.getBusVoltage_V() * 1000.0f;
}

float getRightVoltage_mV() {
  return inaRight.getBusVoltage_V() * 1000.0f;
}

void readVoltages(float &vLeftRaw, float &vRightRaw, float &vLeft, float &vRight) {
  vLeftRaw  = getLeftVoltage_mV();
  vRightRaw = getRightVoltage_mV();
  vLeft     = vLeftRaw;
  vRight    = vRightRaw;
}
