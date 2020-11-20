#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "clock.h"
#include "board.h"
#include "disp.h"
#include "menu.h"
#include "accel.h"
#include "settings.h"
#include "nrf_delay.h"

extern bool buttonPressed;

void settings_time_set(void)
{
	time_struct_t newTime;

	display_clear();
	display_string("set time",0,80);
	nrf_delay_ms(500);

	if(buttonPressed)
	{
		buttonPressed = 0;
		return;  
	}

	// Enable only click interrupt
	acc_int2_sources(I2_CLICK | I2_HLACTIVE);

	display_string("  hours",0,80);
	nrf_delay_ms(500);
	newTime.hours = (uint8_t)numberSelector(12, 0, 23, DISPLAY_DIGITAL);
	nrf_delay_ms(500);

	display_string("  minutes",0,80);
	nrf_delay_ms(500);
	newTime.minutes = (uint8_t)numberSelector(0, 0, 59, DISPLAY_DIGITAL);
	nrf_delay_ms(500);

	display_string("  seconds",0,80);
	nrf_delay_ms(500);
	newTime.seconds = (uint8_t)numberSelector(0, 0, 59, DISPLAY_DIGITAL);
	nrf_delay_ms(500);

	clock_set_time(newTime);
	display_string("  success  ",0,80);
	buttonPressed = 0;

	// Re-enable other interrupt sources
	acc_int2_sources(I2_CLICK | I2_IG1 | I2_HLACTIVE);   // Orientation detection interrupt on output 2
}
