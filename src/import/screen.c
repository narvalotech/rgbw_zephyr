#include <zephyr.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "clock.h"
#include "disp.h"
#include "menu.h"
#include "state.h"
#include "screen.h"

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
