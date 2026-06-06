#ifndef CALIBRATION_H
#define CALIBRATION_H

#include "display.h"

struct CalibrationData {
  float coeff_left;
  float coeff_right;
  uint32_t magic;
};

void initCalibration();
void loadCalibration();
void saveCalibration();
void resetCalibration();

void applyCalibration(float &vLeft, float &vRight);
Mode handleCalibration(Mode modeAfterCalibration, float vLeftRaw, float vRightRaw,
                       float &vLeft, float &vRight);

float getCalibrationLeftCoeff();
float getCalibrationRightCoeff();

#endif // CALIBRATION_H
