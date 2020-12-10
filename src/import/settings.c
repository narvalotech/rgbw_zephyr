#include <zephyr.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "clock.h"
#include "disp.h"
#include "menu.h"
#include "state.h"
#include "settings.h"

extern struct g_state state;

#define SCROLL_SPEED 50

void settings_time_set(void)
{
	time_struct_t newTime;

	display_clear();
	display_string("set time",0,SCROLL_SPEED);
	k_msleep(500);

	if(state.button_pressed)
	{
		state.button_pressed = 0;
		return;  
	}

	display_string("  hours",0,SCROLL_SPEED);
	k_msleep(500);
	newTime.hours = (uint8_t)numberSelector(12, 0, 23, DISPLAY_DIGITAL);
	k_msleep(500);

	display_string("  minutes",0,SCROLL_SPEED);
	k_msleep(500);
	newTime.minutes = (uint8_t)numberSelector(0, 0, 59, DISPLAY_DIGITAL);
	k_msleep(500);

	display_string("  seconds",0,SCROLL_SPEED);
	k_msleep(500);
	newTime.seconds = (uint8_t)numberSelector(0, 0, 59, DISPLAY_DIGITAL);
	k_msleep(500);

	clock_set_time(newTime);
	display_string("  success  ",0,SCROLL_SPEED);
	state.button_pressed = 0;
}
