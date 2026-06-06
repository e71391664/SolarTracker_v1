#ifndef INA226_H
#define INA226_H

#include <INA226_WE.h>

#define INA_LEFT_ADDR  0x40   // левая панель
#define INA_RIGHT_ADDR 0x41   // правая панель

void initINA226();
float getLeftVoltage_mV();
float getRightVoltage_mV();
void readVoltages(float &vLeftRaw, float &vRightRaw, float &vLeft, float &vRight);

#endif // INA226_H
