/*
 * Copyright (c) 2021 Jonathan Rico
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __CTS_H_
#define __CTS_H_

#include <stdbool.h>
#include <zephyr/types.h>

/* Need to pack the structs to be compatible with bluetooth spec */
#ifndef __PACKED
#define __PACKED __attribute__((__packed__))
#endif

struct date_time {
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
}__PACKED;

typedef enum {
	UNKNOWN = 0,
	MONDAY,
	TUESDAY,
	WEDNESDAY,
	THURSDAY,
	FRIDAY,
	SATURDAY,
	SUNDAY,
} day_of_week_t;

struct day_of_week {
	uint8_t day_of_week;
};

struct day_date_time {
	struct date_time date_time;
	struct day_of_week day_of_week;
}__PACKED;

struct exact_time_256 {
	struct day_date_time day_date_time;
	uint8_t fractions256; /* Number of 1/256 fractions of a second */
}__PACKED;

typedef enum {
	MANUAL_TIME_UPDATE = 0,
	EXTERNAL_REFERENCE_TIME_UPDATE,
	CHANGE_OF_TIME_ZONE,
	CHANGE_OF_DST,
} adjust_reason_t;

struct current_time {
	struct exact_time_256 exact_time_256;
	uint8_t adjust_reason;
}__PACKED;

struct current_time *bt_cts_get_current_time(void);

#endif // __CTS_H_
