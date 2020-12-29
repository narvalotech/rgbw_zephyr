#include <zephyr.h>
#include "stopwatch.h"

static uint64_t time = 0;	/* Total time in ms */
static uint8_t cents = 0;
static uint8_t seconds = 0;
static uint8_t minutes = 0;
static stopwatch_state_t state = STW_STOPPED;

struct k_timer stopwatch_timer;
static void stopwatch_timer_callback(struct k_timer *timer_id)
{
	if(state != STW_STOPPED)
	{
		time += 10;
		cents += 1;
		if(cents == 100)
		{
			cents = 0;
			seconds += 1;
		}
		if(seconds == 60)
		{
			seconds = 0;
			minutes += 1;
		}
		if(minutes == 60)
		{
			minutes = 0;
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
	time = 0;
	cents = 0;
	seconds = 0;
	minutes = 0;
	state = STW_STOPPED;
}

stopwatch_state_t stopwatch_state_get(uint8_t channel)
{
	ARG_UNUSED(channel);
	return state;
}

uint32_t stopwatch_ms_get(uint8_t channel)
{
	ARG_UNUSED(channel);
	return time;
}

uint8_t stopwatch_cents_get(uint8_t channel)
{
	ARG_UNUSED(channel);
	return cents;
}

uint8_t stopwatch_seconds_get(uint8_t channel)
{
	ARG_UNUSED(channel);
	return seconds;
}

uint8_t stopwatch_minutes_get(uint8_t channel)
{
	ARG_UNUSED(channel);
	return minutes;
}
