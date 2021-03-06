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
#include "battery.h"
#include "ble.h"
#include "metronome.h"
#include "motor.h"

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

	uint32_t arm_reset = 0;

	int i = 0;
	while(!state.exit_signal && !state.main)
	{
		i++;
		if(i >= SLEEP_TIMEOUT)
		{
			i = 0;
			board_suspend();
		}
		if(state.but_ur == 2)
		{
			state.but_ur = 0;
			if(!arm_reset) {
				arm_reset = 1;
			}
			else {
				arm_reset = 0;
			}
		}
		if(state.but_lr == 2)
		{
			if(!arm_reset) {
				state.but_lr = 0;
				state.pgm_state = PGM_STATE_CLOCK_SET;
				break;
			}
			arm_reset = 2;
		}
		display_bcd(p_time->hours, p_time->minutes, p_time->seconds, 0);
		clock_thread_sync();
	}

	if(arm_reset == 2) {
		NVIC_SystemReset();
	}

	state_clear();
}

void screen_stopwatch(void)
{
	uint32_t i = 0;

	display_clear();
	display_mono_set_color(0, 255, 0);
	display_string("stw", 0, SCROLL_SPEED);
	k_msleep(DISP_DELAY);

	while(!state.exit_signal && !state.main)
	{
		i++;
		if(i >= SLEEP_TIMEOUT * 100)
		{
			i = 0;
			board_suspend();
		}

		if(state.but_ur)
		{
			/* Start/stop */
			state.but_ur = 0;
			stopwatch_toggle(0);
		}
		if(state.but_lr == 1)
		{
			/* Clear counter
			 * Just wakes the screen if stopwatch
			 * counter is not stopped */
			state.but_lr = 0;
			if(stopwatch_state_get(0) == STW_STOPPED)
			{
				stopwatch_reset(0);
				display_clear();
			}
		}

		display_bcd(stopwatch_minutes_get(0),
			stopwatch_seconds_get(0),
			stopwatch_cents_get(0),
			0);

		/* Wait for either next timer tick or button
		 * interrupt */
		if(stopwatch_state_get(0) == STW_STARTED) {
			stopwatch_thread_sync(0);
		}
		else {
			k_msleep(10);
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
#define BATT_NODE DT_NODELABEL(batmon_en)
#define BATT_PIN DT_GPIO_PIN(BATT_NODE, gpios)
#define MOTOR_NODE DT_NODELABEL(hapt_gpio)
#define MOTOR_PIN DT_GPIO_PIN(MOTOR_NODE, gpios)

	uint8_t status_batt = 0;
	uint8_t status_motor = 0;
	const struct device *batt;
	const struct device *motor;

	batt = device_get_binding(DT_GPIO_LABEL(BATT_NODE, gpios));
	motor = device_get_binding(DT_GPIO_LABEL(MOTOR_NODE, gpios));

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

static const struct battery_level_point levels[] = {
/* See batt sample for explanation */
	{ 10000, 3950 },
	{ 625, 3550 },
	{ 0, 3100 },
};

void screen_battery(void)
{
	/* Display battery level as percent and bargraph */
	int batt_mv = 0;

	/* Remove load to get more accurate reading */
	board_enable_5v(0);
	battery_measure_enable(true);
	k_msleep(100);

	/* Get batt level */
	while(!batt_mv && !state.exit_signal && !state.main)
	{
		batt_mv = battery_sample();
	}
	/* Gives weird-ass values */
	unsigned int batt_percent = battery_level_pptt(batt_mv, levels) / 10;
	battery_measure_enable(false);

	board_enable_5v(1);
	display_clear();
	display_mono_set_color(255, 3, 209); /* Pink */
	display_string("batt", 0, SCROLL_SPEED);
	k_msleep(DISP_DELAY);

	/* Display batt level */
	/* TODO: add bargraph */
	display_bcd(0, batt_percent, 0, 0);

	/* Wait 5s and exit to next screen */
	if(!state.exit_signal && !state.main)
		k_sleep(K_SECONDS(5));

	state_clear();
	display_clear();
}

void screen_ble(void)
{
	display_clear();
	display_mono_set_color(0, 0, 255); /* Blue */
	display_string("bluetooth", 0, SCROLL_SPEED);
	k_msleep(DISP_DELAY);

	while(!state.exit_signal && !state.main)
	{
		if(state.but_ur == 1)
		{
			state.but_ur = 0;
			board_enable_5v(1);
			display_string("adv start", 0, SCROLL_SPEED);
			k_msleep(DISP_DELAY);
			ble_adv(1);
			board_enable_5v(0);
		}
		if(state.but_lr == 1)
		{
			state.but_lr = 0;
			board_enable_5v(1);
			display_string("adv stop", 0, SCROLL_SPEED);
			k_msleep(DISP_DELAY);
			ble_adv(0);
			board_enable_5v(0);
		}
		if (state.but_ur == 2)
		{
			state.but_ur = 0;
			board_enable_5v(1);
			display_string("update param", 0, SCROLL_SPEED);
			k_msleep(DISP_DELAY);
			ble_adv(0);
			board_enable_5v(0);
			ble_update_param();
		}

		/* Wait for next IRQ */
		k_sleep(K_FOREVER);
	}

	/* Restart display for next screen */
	board_enable_5v(1);
	state_clear();
	display_clear();
}

void screen_metronome(void)
{
	display_clear();
	display_mono_set_color(128, 29, 214); /* Purple-ish */
	display_string("metronome", 0, SCROLL_SPEED);
	k_msleep(DISP_DELAY);

	static uint32_t tempo_bpm = 120;
	uint8_t enable = 0;

	metronome_set_tempo(tempo_bpm);

	while(!state.exit_signal && !state.main)
	{
		if(state.but_ur)
		{
			state.but_ur = 0;
			metronome_tap_tempo();
		}
		if(state.but_lr)
		{
			state.but_lr = 0;
			enable ^= 0x01;
			metronome_enable(enable);
		}

		tempo_bpm = metronome_get_tempo();
		uint32_t top = tempo_bpm / 100;
		uint32_t mid = tempo_bpm % 100;
		display_bcd(top, mid, enable, 0);

		/* Wait for next IRQ */
		k_sleep(K_FOREVER);
	}

	metronome_enable(false);
	display_clear();
	state_clear();
}
