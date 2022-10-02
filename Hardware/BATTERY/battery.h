#ifndef __BATTERY_H
#define __BATTERY_H

#include "stm32f10x.h"

void AD_Init(void);
uint16_t AD_GetValue(void);
float AD_GetVoltage(void);
u8 ReturnBatteryStatus(void);
void Charge_Init(void);
u8 ReturnChargeStatus(void);


#endif /* __BATTERY */

