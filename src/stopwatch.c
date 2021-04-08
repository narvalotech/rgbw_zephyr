#include <zephyr.h>
#include <string.h>
#include "stopwatch.h"

static stopwatch_time_t time;
static stopwatch_state_t state = STW_STOPPED;
struct k_timer stopwatch_timer;

static void stopwatch_timer_callback(struct k_timer *timer_id)
{
	if(state != STW_STOPPED)
	{
		time.ms += 10;
		time.cents += 1;
		if(time.cents == 100)
		{
			time.cents = 0;
			time.seconds += 1;
		}
		if(time.seconds == 60)
		{
			time.seconds = 0;
			time.minutes += 1;
		}
		if(time.minutes == 60)
		{
			time.minutes = 0;
			time.hours += 1;
		}
		if(time.hours > 99)
		{
			/* Can't display more than 99 in BCD mode */
			time.hours = 0;
		}
	}
}

K_TIMER_DEFINE(stopwatch_timer, stopwatch_timer_callback, NULL);

void stopwatch_init(uint8_t channel)
{
	ARG_UNUSED(channel);
	stopwatch_reset(0);
}

void stopwatch_thread_sync(uint8_t channel)
{
	ARG_UNUSED(channel);
	k_timer_status_sync(&stopwatch_timer);
}

void stopwatch_start(uint8_t channel)
{
	ARG_UNUSED(channel);
	state = STW_STARTED;
	k_timer_start(&stopwatch_timer, K_MSEC(10), K_MSEC(10));
}

void stopwatch_stop(uint8_t channel)
{
	ARG_UNUSED(channel);
	state = STW_STOPPED;
	k_timer_stop(&stopwatch_timer);
}

stopwatch_state_t stopwatch_toggle(uint8_t channel)
{
	if(state == STW_STOPPED)
		stopwatch_start(channel);
	else
		stopwatch_stop(channel);

	return state;
}

void stopwatch_reset(uint8_t channel)
{
	ARG_UNUSED(channel);
	memset(&time, 0, sizeof(time));
	state = STW_STOPPED;
}

stopwatch_state_t stopwatch_state_get(uint8_t channel)
{
	ARG_UNUSED(channel);
	return state;
}

stopwatch_time_t* stopwatch_time_get(uint8_t channel)
{
	ARG_UNUSED(channel);
	return &time;
}

uint32_t stopwatch_ms_get(uint8_t channel)
{
	ARG_UNUSED(channel);
	return time.ms;
}

uint8_t stopwatch_cents_get(uint8_t channel)
{
	ARG_UNUSED(channel);
	return time.cents;
}

uint8_t stopwatch_seconds_get(uint8_t channel)
{
	ARG_UNUSED(channel);
	return time.seconds;
}

uint8_t stopwatch_minutes_get(uint8_t channel)
{
	ARG_UNUSED(channel);
	return time.minutes;
}

uint8_t stopwatch_hours_get(uint8_t channel)
{
	ARG_UNUSED(channel);
	return time.hours;
}
