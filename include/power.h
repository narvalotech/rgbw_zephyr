#ifndef __POWER_H_
#define __POWER_H_

#include "stdbool.h"

void power_system_sleep();
void power_system_wakeup();
void power_periph_shutdown();
void power_periph_wakeup();
void power_hibernate(bool initialized);

void led_vdd_enable(bool enable);
void sensor_vdd_enable(bool enable);

#endif // __POWER_H_
