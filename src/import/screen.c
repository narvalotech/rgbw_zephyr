#include <zephyr.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include "screen.h"
#include "clock.h"
#include "disp.h"
#include "menu.h"
#include "state.h"
#include "stopwatch.h"
#include "accel.h"
#include "board.h"

extern struct g_state state;

#define SCROLL_SPEED 50
#define DISP_DELAY 200
#define SLEEP_TIMEOUT 10

void screen_time_set(void)
{
	time_struct_t newTime;

	display_mono_set_color(255, 0, 0);
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
	display_mono_set_color(255, 160, 0);

	/* Live time value pointer */
	time_struct_t* p_time = clock_get_time_p();

	display_clear();
	display_string("clock", 0, SCROLL_SPEED);

	int i = 0;

	while(!state.exit_signal && !state.main)
	{
		i++;
		if(i >= SLEEP_TIMEOUT)
		{
			i = 0;
			board_suspend();
		}
		display_bcd(p_time->hours, p_time->minutes, p_time->seconds, 0);
		clock_thread_sync();
	}

	state_clear();
}

void screen_stopwatch(void)
{
	display_clear();
	display_mono_set_color(0, 255, 0);
	display_string("stw", 0, SCROLL_SPEED);
	k_msleep(DISP_DELAY);

	while(!state.exit_signal && !state.main)
	{
		if(state.but_ur)
		{
			state.but_ur = 0;
			stopwatch_toggle(0);
		}
		if(state.but_lr == 1)
		{
			state.but_lr = 0;
			if(stopwatch_state_get(0) == STW_STOPPED)
			{
				stopwatch_reset(0);
				display_clear();
			}
		}

		if(stopwatch_state_get(0) == STW_STARTED)
		{
			display_bcd(stopwatch_minutes_get(0),
				stopwatch_seconds_get(0),
				stopwatch_cents_get(0),
				0);
			stopwatch_thread_sync(0);
		}
		else {
			k_sleep(K_MSEC(10));
		}
	}

	state_clear();
	display_clear();
}

void screen_test_tilt(void)
{
	#define TEST_TILT_THRESHOLD 300

	uint8_t display[4] = {0};
	int32_t accel[3];

	display_mono_set_color(86, 213, 245);

	while(!state.exit_signal && !state.main)
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

void screen_test(void)
{
	/* Test HW functions:
	 * - battery voltage divider enable
	 * - motor driver enable */
	uint8_t status_batt = 0;
	uint8_t status_motor = 0;
	const struct device *batt;
	const struct device *motor;

#define BATT_NODE DT_NODELABEL(batmon_en)
#define BATT_PIN DT_GPIO_PIN(BATT_NODE, gpios)

	batt = device_get_binding(DT_GPIO_LABEL(BATT_NODE, gpios));
	gpio_pin_configure(batt, BATT_PIN, GPIO_OUTPUT | DT_GPIO_FLAGS(BATT_NODE, gpios));

#define MOTOR_NODE DT_NODELABEL(hapt_pwm)
#define MOTOR_PIN DT_GPIO_PIN(MOTOR_NODE, gpios)

	motor = device_get_binding(DT_GPIO_LABEL(MOTOR_NODE, gpios));
	gpio_pin_configure(motor, MOTOR_PIN, GPIO_OUTPUT | DT_GPIO_FLAGS(MOTOR_NODE, gpios));

	display_clear();
	display_mono_set_color(152, 6, 237); /* Purple */
	display_string("test", 0, SCROLL_SPEED);
	k_msleep(DISP_DELAY);

	while(!state.exit_signal && !state.main)
	{
		if(state.but_ur == 1)
		{
			state.but_ur = 0;
			/* Toggle battery connection */
			status_batt ^= 0x01;
			gpio_pin_set(batt, BATT_PIN, status_batt);
		}
		if(state.but_lr == 1)
		{
			state.but_lr = 0;
			/* Toggle motor driver GPIO */
			status_motor ^= 0x01;
			gpio_pin_set(motor, MOTOR_PIN, status_motor);
		}

		display_bytes(0x88, status_batt, status_motor, 0);

		/* Wait for next IRQ */
		k_sleep(K_FOREVER);
	}

	state_clear();
	display_clear();
}
