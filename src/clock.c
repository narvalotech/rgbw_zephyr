#include <stdint.h>
#include <stddef.h>
#include <zephyr.h>
#include "calendar.h"
#include "alarm.h"
#include "clock.h"

time_struct_t currentTime;

static uint32_t clock_period_s = 1;

K_SEM_DEFINE(wait_on_tick, 0, 1);

struct k_timer clock_timer;
static void clock_timer_callback(struct k_timer *timer_id)
{
	clock_increment_seconds(clock_period_s);
	k_sem_give(&wait_on_tick);
}

K_TIMER_DEFINE(clock_timer, clock_timer_callback, NULL);

void clock_thread_sync(void)
{
	k_sem_take(&wait_on_tick, K_FOREVER);
}

void clock_thread_unblock(void)
{
	k_sem_give(&wait_on_tick);
}

void clock_set_high_latency(bool latency)
{
	if(latency) {
		/* Switch to 60s latency */
		k_timer_status_sync(&clock_timer);
		clock_period_s = 60;
		k_timer_start(&clock_timer,
			      K_SECONDS(clock_period_s),
			      K_SECONDS(clock_period_s));
	}
	else {
		/* Recover remaining time */
		uint32_t rem_sec = k_timer_remaining_get(&clock_timer) / 1000;
		clock_increment_seconds(60 - rem_sec);
		/* Switch to 1s latency */
		clock_period_s = 1;
		k_timer_start(&clock_timer,
			      K_SECONDS(clock_period_s),
			      K_SECONDS(clock_period_s));
	}
}

void clock_time_init()
{
	currentTime.hours   = 12;
	currentTime.minutes = 00;
	currentTime.seconds = 00;

	k_timer_start(&clock_timer, K_SECONDS(1), K_SECONDS(1));
}

time_struct_t* clock_get_time_p()
{
	return (time_struct_t*)(&currentTime);
}

void clock_set_time(time_struct_t newTime)
{
	currentTime.hours   = newTime.hours;
	currentTime.minutes = newTime.minutes;
	currentTime.seconds = newTime.seconds;
}

void clock_increment_seconds(uint32_t seconds)
{
	for(; seconds>0; seconds--) {
		currentTime.seconds++;
		if(currentTime.seconds == 60)
		{
			currentTime.minutes++;
			currentTime.seconds = 0;

			if(currentTime.minutes == 60)
			{
				currentTime.hours++;
				currentTime.minutes = 0;
			}
			if(currentTime.hours == 24) {
				currentTime.hours = 0;
				cal_increment_day();
			}

			alarm_check();
		}
	}
}
