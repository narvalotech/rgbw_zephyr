#include <string.h>
#include "calendar.h"

static bool leap_year = false;
static struct date_time curr_date;
const uint8_t days_in_month[12] = { 31, /* Jan */
				    28, /* Feb */
				    31, /* Mar */
				    30, /* Apr */
				    31, /* May */
				    30, /* Jun */
				    31, /* Jul */
				    31, /* Aug */
				    30, /* Sep */
				    31, /* Oct */
				    30, /* Nov */
				    31 }; /* Dec */

const char cal_month_string[12][4] = {
	"jan", "feb", "mar", "apr", "may", "jun",
	"jul", "aug", "sep", "oct", "nov", "dec"
};

static void check_leap_year(void)
{
	/* Extra feb day for leap years */
	/* Leap year:
	 * - divisible by 4
	 * - except when divisible by 100 and not by 400
	 */
	if ((curr_date.year % 4) &&
	   !(curr_date.year % 100 && !(curr_date.year % 400)))
	{
		leap_year = true;
	} else {
		leap_year = false;
	}
}

static void constrain_date(void)
{
	uint8_t days_in_curr_month = days_in_month[curr_date.month - 1];

	/* Extra day in february when in a leap year */
	if(leap_year && curr_date.month == 1) {
		days_in_curr_month += 1;
	}

	if(curr_date.day < 1) {
		curr_date.day = 1;
	} else if(curr_date.day > days_in_curr_month)
	{
		curr_date.day = days_in_curr_month;
	}

	if(curr_date.month < 1)
	{
		curr_date.month = 1;
	} else if(curr_date.month > 12)
	{
		curr_date.month = 12;
	}

	if(curr_date.year < 2000)
	{
		curr_date.year = 2000;
	} else if(curr_date.year > 3000)
	{
		/* If this still works in a 1000 yeers I'll be lucky */
		curr_date.year = 3000;
	}
}

void cal_set_date(struct date_time* p_date)
{
	memcpy(&curr_date, p_date, sizeof(curr_date));
	check_leap_year();
	constrain_date();
}

void cal_get_date(struct date_time* p_date)
{
	memcpy(p_date, &curr_date, sizeof(curr_date));
}

struct date_time* cal_get_date_ptr(void)
{
	return &curr_date;
}

void cal_increment_day(void)
{
	uint8_t days_in_curr_month = days_in_month[curr_date.month - 1];

	/* Extra day in february when in a leap year */
	if(leap_year && curr_date.month == 1) {
		days_in_curr_month += 1;
	}

	if(curr_date.day < days_in_curr_month)
	{
		curr_date.day += 1;
	} else {
		curr_date.day = 1;
		if(curr_date.month < 12) {
			curr_date.month += 1;
		} else {
			curr_date.month = 1;
			curr_date.year += 1;
			check_leap_year();
		}
	}
}

uint8_t cal_get_month(void)
{
	return curr_date.month;
}

uint8_t cal_get_day(void)
{
	return curr_date.day;
}

uint16_t cal_get_year(void)
{
	return curr_date.year;
}

/* Utility functions */

/* Calculate day of week in proleptic Gregorian calendar. Sunday == 0. */
/* Taken from https://rosettacode.org/wiki/Day_of_the_week#C */
day_of_week_t cal_get_weekday(struct date_time* p_date)
{
	int adjustment = 0;
	int mm = 0;
	int yy = 0;

	if(p_date == NULL) {
		p_date = &curr_date;
	}

	adjustment = (14 - p_date->month) / 12;
	mm = p_date->month + 12 * adjustment - 2;
	yy = p_date->year - adjustment;

	int dow = (p_date->day
		   + (13 * mm - 1) / 5
		   + yy
		   + yy / 4
		   - yy / 100
		   + yy / 400) % 7;

	if(dow == 0) {		/* sunday is at the end of day_of_week_t */
		dow = 7;
	}

	return dow;
}
