#ifndef ALARM_H_
#define ALARM_H_

#include "clock.h"		/* For time_struct_t */

typedef struct
{
	time_struct_t time;
	time_struct_t time_sn;	/* Snooze-adjusted time */
	uint8_t days;
} alarm_struct_t;

/* days: bitmap with day_of_week_t bit-ordering */
void alarm_set(alarm_struct_t * p_alarm_time);
void alarm_get(alarm_struct_t * p_alarm_time);

/* Call to check if current time is defined as an alarm */
bool alarm_check(void);
void alarm_enable(bool enable);
bool alarm_is_enabled(void);
void alarm_snooze(uint8_t minutes);
void alarm_stop(void);
void alarm_start(void);

/* Missing:
 * - set alarm time
 * - set alarm days
 * - enable/disable alarm
 * - hook alarm module to time module
 * - make alarm ring (goto defined screen)
 * - enable motor/vibration on alarm expiry */

#endif // ALARM_H_
