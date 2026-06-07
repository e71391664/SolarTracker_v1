#include <EEPROM.h>
#include "calibration.h"

extern bool calibrationComboLockout;
extern String statusMessage;

#define EEPROM_CALIB_ADDR 0
#define EEPROM_SIZE       512
#define CALIB_MAGIC       0xCA11B0A

static CalibrationData calibration = { 1.0f, 1.0f, CALIB_MAGIC };

// Инициализация калибровки (загрузка из EEPROM)
void initCalibration() {
  loadCalibration();
}

// Получение калиброванных напряжений
void loadCalibration() {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(EEPROM_CALIB_ADDR, calibration);
  EEPROM.end();

  if (calibration.magic != CALIB_MAGIC) {
    resetCalibration();
    Serial.println("⚠ Калибровка не найдена, используются коэффициенты по умолчанию");
  } else {
    Serial.printf("✓ Калибровка загружена: L=%.4f, R=%.4f\n",
                  calibration.coeff_left, calibration.coeff_right);
  }
}

// Сохранение калибровки в EEPROM
void saveCalibration() {
  calibration.magic = CALIB_MAGIC;
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.put(EEPROM_CALIB_ADDR, calibration);
  EEPROM.commit();
  EEPROM.end();
  Serial.printf("✓ Калибровка сохранена: L=%.4f, R=%.4f\n",
                calibration.coeff_left, calibration.coeff_right);
}

// Сброс калибровки на значения по умолчанию
void resetCalibration() {
  calibration.coeff_left = 1.0f;
  calibration.coeff_right = 1.0f;
  calibration.magic = CALIB_MAGIC;
  saveCalibration();
  Serial.println("✓ Калибровка сброшена на значения по умолчанию");
}
// Применение калибровки к сырым напряжениям
void applyCalibration(float &vLeft, float &vRight) {
  vLeft  *= calibration.coeff_left;
  vRight *= calibration.coeff_right;
}

// Обработка калибровки (вызывается при удержании обеих кнопок)
Mode handleCalibration(Mode modeAfterCalibration, float vLeftRaw, float vRightRaw,
                       float &vLeft, float &vRight) {
  if (vLeftRaw <= 1.0f || vRightRaw <= 1.0f) {
    Serial.println("Калибровка отменена: слишком малое напряжение на одном из каналов.");
    statusMessage = "Калибровка отменена: недостаточное напряжение";
    calibrationComboLockout = false;
    return modeAfterCalibration;
  }

  float target = (vLeftRaw > vRightRaw) ? vLeftRaw : vRightRaw;

  calibration.coeff_left = target / vLeftRaw;
  calibration.coeff_right = target / vRightRaw;

  vLeft = vLeftRaw;
  vRight = vRightRaw;
  applyCalibration(vLeft, vRight);

  Serial.printf("\n=== КАЛИБРОВКА ЗАВЕРШЕНА ===\n");
  Serial.printf("RAW: L=%.1f mV  R=%.1f mV  diff=%.1f mV\n",
                vLeftRaw, vRightRaw, vLeftRaw - vRightRaw);
  Serial.printf("CAL: L=%.1f mV  R=%.1f mV  diff=%.1f mV\n",
                vLeft, vRight, vLeft - vRight);
  Serial.printf("Коэффициенты: L=%.4f  R=%.4f\n",
                calibration.coeff_left, calibration.coeff_right);

  saveCalibration();
  statusMessage = "Калибровка завершена успешно";
  calibrationComboLockout = false;
  return modeAfterCalibration;
}

// Получение текущих коэффициентов калибровки
float getCalibrationLeftCoeff() {
  return calibration.coeff_left;
}

//
float getCalibrationRightCoeff() {
  return calibration.coeff_right;
}
