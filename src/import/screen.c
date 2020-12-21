#include <zephyr.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "clock.h"
#include "disp.h"
#include "menu.h"
#include "state.h"
#include "screen.h"
#include "stopwatch.h"

extern struct g_state state;

#define SCROLL_SPEED 50
#define DISP_DELAY 200

void screen_time_set(void)
{
	time_struct_t newTime;

	display_clear();
	display_string("set time",0,SCROLL_SPEED);
	k_msleep(DISP_DELAY);

	if(state.exit_signal)
	{
		state.exit_signal = 0;
		return;  
	}

	display_string("  hours",0,SCROLL_SPEED);
	k_msleep(DISP_DELAY);
	newTime.hours = (uint8_t)numberSelector(12, 0, 23, DISPLAY_DIGITAL);
	if(state.main)
		return;
	k_msleep(DISP_DELAY);

	display_string("  minutes",0,SCROLL_SPEED);
	k_msleep(DISP_DELAY);
	newTime.minutes = (uint8_t)numberSelector(0, 0, 59, DISPLAY_DIGITAL);
	if(state.main)
		return;
	k_msleep(DISP_DELAY);

	display_string("  seconds",0,SCROLL_SPEED);
	k_msleep(DISP_DELAY);
	newTime.seconds = (uint8_t)numberSelector(0, 0, 59, DISPLAY_DIGITAL);
	if(state.main)
		return;
	k_msleep(DISP_DELAY);

	clock_set_time(newTime);
	display_string("  success  ",0,SCROLL_SPEED);
}

void screen_stopwatch(void)
{
	uint32_t time = 0;
	display_clear();
	display_string("stopwatch", 0, SCROLL_SPEED);
	k_msleep(DISP_DELAY);

	while(state.exit_signal || state.main)
	{
		if(state.but_ll)
		{
			state.but_ll = 0;
			stopwatch_toggle(0);
		}
		if(state.but_ur)
		{
			state.but_ur = 0;
			if(stopwatch_state_get(0) == STW_STOPPED)
			{
				stopwatch_reset(0);
			}
		}

		time = stopwatch_ms_get(0);
		display_bcd(time / (60 * 60 * 1000), /* Hours */
			    time / (60 * 1000),	     /* Minutes */
			    time / 1000, 0);	     /* Seconds */
		stopwatch_thread_sync(0);
	}

	state.select = 0;
	state.exit_signal = 0;
	display_clear();
}
