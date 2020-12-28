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
#include "accel.h"

extern struct g_state state;

#define SCROLL_SPEED 50
#define DISP_DELAY 200

void screen_time_set(void)
{
	time_struct_t newTime;

	display_clear();
	display_string("set time",0,SCROLL_SPEED);
	k_msleep(DISP_DELAY);

	if(state.exit_signal || state.main)
	{
		state_clear();
		return;  
	}

	display_string("  hours",0,SCROLL_SPEED);
	k_msleep(DISP_DELAY);
	newTime.hours = (uint8_t)numberSelector(12, 0, 23, DISPLAY_DIGITAL);
	if(state.main)
	{
		state_clear();
		return;
	}
	k_msleep(DISP_DELAY);

	display_string("  minutes",0,SCROLL_SPEED);
	k_msleep(DISP_DELAY);
	newTime.minutes = (uint8_t)numberSelector(0, 0, 59, DISPLAY_DIGITAL);
	if(state.main)
	{
		state_clear();
		return;
	}
	k_msleep(DISP_DELAY);

	display_string("  seconds",0,SCROLL_SPEED);
	k_msleep(DISP_DELAY);
	newTime.seconds = (uint8_t)numberSelector(0, 0, 59, DISPLAY_DIGITAL);
	if(state.main)
	{
		state_clear();
		return;
	}
	k_msleep(DISP_DELAY);

	clock_set_time(newTime);
	display_string("  success  ",0,SCROLL_SPEED);

	state_clear();
}

void screen_clock_bcd(void)
{
	/* Live time value pointer */
	time_struct_t* p_time = clock_get_time_p();

	while(state.exit_signal || state.main)
	{
		display_bcd(p_time->hours, p_time->minutes, p_time->seconds, 0);
		clock_thread_sync();
	}

	state_clear();
}

void screen_stopwatch(void)
{
	uint32_t time = 0;
	display_clear();
	display_string("stopwatch", 0, SCROLL_SPEED);
	k_msleep(DISP_DELAY);

	while(state.exit_signal || state.main)
	{
		if(state.but_ur)
		{
			state.but_ur = 0;
			stopwatch_toggle(0);
		}
		if(state.but_ll == 1)
		{
			state.but_ll = 0;
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

	state_clear();
	display_clear();
}

void screen_test_tilt(void)
{
	#define TEST_TILT_THRESHOLD 300

	uint8_t display[4] = {0};
	int32_t accel[3];

	while(state.exit_signal || state.main)
	{
		accel_get_mg(accel);

		if(accel[0] < -TEST_TILT_THRESHOLD) {
			display[0] = 0b01100000;
			display[1] = 0b01100000;
			display[2] = 0b01100000;
		}
		else if(accel[0] > TEST_TILT_THRESHOLD) {
			display[0] = 0b00000110;
			display[1] = 0b00000110;
			display[2] = 0b00000110;
		}
		else {
			display[0] = 0b00011000;
			display[1] = 0b00011000;
			display[2] = 0b00011000;
		}

		if(accel[1] > TEST_TILT_THRESHOLD) {
			display[1] = 0;
			display[2] = 0;
		}
		else if(accel[1] < -TEST_TILT_THRESHOLD) {
			display[0] = 0;
			display[1] = 0;
		}
		else {
			display[0] = 0;
			display[2] = 0;
		}

		display_bytes(display[0], display[1], display[2], 0);
		k_msleep(100);
	}

	state_clear();
	display_clear();
}
