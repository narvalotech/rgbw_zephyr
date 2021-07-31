#ifndef ALARM_H_
#define ALARM_H_

#include "clock.h"		/* For time_struct_t */

/* days: bitmap with day_of_week_t bit-ordering */
void alarm_set(time_struct_t * time, uint8_t days);

void alarm_get_time(time_struct_t * time);

uint8_t alarm_get_days(void);

void alarm_enable(bool enable);
void alarm_snooze(uint8_t minutes);
void alarm_stop();

#endif // ALARM_H_
