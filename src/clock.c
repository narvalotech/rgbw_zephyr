#include <stdint.h>
#include <stddef.h>
#include <zephyr.h>
#include "calendar.h"
#include "alarm.h"
#include "clock.h"
#include "state.h"

extern struct g_state state;

time_struct_t currentTime;
time_struct_t otherTime;
int otherTimeDiff;

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

void clock_set_other_time_diff(uint8_t idx, int hours)
{
	otherTimeDiff = hours;
}

time_struct_t* clock_get_time_p(uint8_t idx)
{
	if(idx == 0)
		return (time_struct_t*)(&currentTime);
	else
		return (time_struct_t*)(&otherTime);
}

void clock_set_time(time_struct_t newTime)
{
	currentTime.hours   = newTime.hours;
	currentTime.minutes = newTime.minutes;
	currentTime.seconds = newTime.seconds;
}

/* Find some other place for this */
static void brightness_adjust(void)
{
	if(currentTime.hours == 21
	   && currentTime.minutes == 0) {
		state.brightness = BRIGHTNESS_NIGHT;
	}
	else if(currentTime.hours == 7
		&& currentTime.minutes == 0) {
		state.brightness = BRIGHTNESS_DAY;
	}
}

static void set_other_time(time_struct_t * p_new_time,
			   time_struct_t * p_time,
			   int hour_diff)
{
	p_new_time->hours = (uint8_t)
		((int)
		 ((int)p_time->hours + (int)hour_diff)
		 % 24);
	p_new_time->minutes = p_time->minutes;
	p_new_time->seconds = p_time->seconds;
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
			brightness_adjust();
		}
	}

	set_other_time(&otherTime, &currentTime, otherTimeDiff);
}
