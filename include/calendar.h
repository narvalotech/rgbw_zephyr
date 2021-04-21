#ifndef __CALENDAR_H_
#define __CALENDAR_H_

#include "cts.h"

extern const char cal_month_string[12][4];

void cal_increment_day(void);
void cal_set_date(struct date_time*);

void cal_get_date(struct date_time*);
struct date_time* cal_get_date_ptr(void);
uint16_t cal_get_year(void);
uint8_t cal_get_month(void);
uint8_t cal_get_day(void);

day_of_week_t cal_get_weekday(struct date_time*);

#endif // __CALENDAR_H_
