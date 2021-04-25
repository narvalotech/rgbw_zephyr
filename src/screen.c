#include <zephyr.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
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
#include "motor.h"
#include "countdown.h"
#include "cts.h"
#include "calendar.h"

extern struct g_state state;

#define SCROLL_SPEED 50
#define DISP_DELAY 200
#define SLEEP_TIMEOUT 10

static int set_date(struct date_time* p_date)
{
	struct date_time date;
	memset(&date, 0, sizeof(date));

	display_clear();
	display_string("set date",0,SCROLL_SPEED);
	k_msleep(DISP_DELAY);

	if(state.exit_signal || state.main)
	{
		return -1;
	}

	display_string("  year", 0, SCROLL_SPEED);
	k_msleep(DISP_DELAY);
	date.year = 2000 + numberSelector(p_date->year, 0, 99, DISPLAY_DIGITAL);
	if(state.main)
	{
		return -1;
	}
	k_msleep(DISP_DELAY);

	display_string("  month", 0, SCROLL_SPEED);
	k_msleep(DISP_DELAY);
	date.month = numberSelector(p_date->month, 1, 12, DISPLAY_DIGITAL);
	if(state.main)
	{
		return -1;
	}
	k_msleep(DISP_DELAY);

	display_string("  day", 0, SCROLL_SPEED);
	k_msleep(DISP_DELAY);
	date.day = numberSelector(p_date->day, 1, 31, DISPLAY_DIGITAL);
	if(state.main)
	{
		return -1;
	}
	k_msleep(DISP_DELAY);

	/* If we were not aborted during whole selection process
	 * then save selection. */
	p_date->year = date.year;
	p_date->month = date.month;
	p_date->day = date.day;

	return 0;
}

static int set_time(time_struct_t* p_time)
{
	time_struct_t time = {0, 0, 0};

	display_clear();
	display_string("set time",0,SCROLL_SPEED);
	k_msleep(DISP_DELAY);

	if(state.exit_signal || state.main)
	{
		return -1;
	}

	display_string("  hours",0,SCROLL_SPEED);
	k_msleep(DISP_DELAY);
	time.hours = (uint8_t)numberSelector(p_time->hours, 0, 23, DISPLAY_DIGITAL);
	if(state.main)
	{
		return -1;
	}
	k_msleep(DISP_DELAY);

	display_string("  minutes",0,SCROLL_SPEED);
	k_msleep(DISP_DELAY);
	time.minutes = (uint8_t)numberSelector(p_time->minutes, 0, 59, DISPLAY_DIGITAL);
	if(state.main)
	{
		return -1;
	}
	k_msleep(DISP_DELAY);

	display_string("  seconds",0,SCROLL_SPEED);
	k_msleep(DISP_DELAY);
	time.seconds = (uint8_t)numberSelector(p_time->seconds, 0, 59, DISPLAY_DIGITAL);
	if(state.main)
	{
		return -1;
	}
	k_msleep(DISP_DELAY);

	/* If we were not aborted during whole selection process
	 * then save selection. */
	p_time->hours = time.hours;
	p_time->minutes = time.minutes;
	p_time->seconds = time.seconds;

	return 0;
}

void screen_time_set(void)
{
	time_struct_t newTime;

	/* Start selector at current time */
	memcpy(&newTime, clock_get_time_p(), sizeof(newTime));

	display_mono_set_color(255, 0, 0);
	if(!set_time(&newTime))
	{
		clock_set_time(newTime);
		display_string("  success  ",0,SCROLL_SPEED);
	}

	state_clear();
}

void screen_clock(void)
{
	display_mono_set_color(255, 160, 0);

	/* Live time value pointer */
	time_struct_t* p_time = clock_get_time_p();
	char date_buf[30] = {0};

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
			/* Dim leds before turning off */
			display_fade_next(DISP_FX_DIR_OUT, 500, DISP_FX_FADE);
			board_suspend();
		}

		if(state.but_ur == 1) {
			state.but_ur = 0;
			if(i == 0) {
				/* When waking up from sleep, display time in BCD */
				display_bcd(p_time->hours,
					    p_time->minutes,
					    p_time->seconds, 0);
				clock_thread_sync();
			} else {
				/* If already woken up, display current date */
				sprintf(date_buf, "  %d %s %d  ",
					cal_get_day(),
					cal_month_string[cal_get_month() - 1],
					cal_get_year());
				display_string(date_buf, 0, 80);
				/* Go to sleep right away */
				i = SLEEP_TIMEOUT;
				if(state.but_ur) {
					/* If user pressed button during date display,
					 * then set the date. */
					struct date_time date;
					if(set_date(&date) == 0) {
						cal_set_date(&date);
					}
				}
				display_clear();
				continue;
			}
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
			state.but_lr = 0;
			if(!arm_reset) {
				state.pgm_state = PGM_STATE_CLOCK_SET;
				break;
			}
			arm_reset = 2;
		}

		if(state.but_lr) {
			/* Display digital time when user presses lower-right button */
			state.but_lr = 0;
			display_fade_next(DISP_FX_DIR_IN, 500, DISP_FX_FADE);
			display_number(p_time->hours, 0);
			k_msleep(1000);
			display_clear();

			display_fade_next(DISP_FX_DIR_IN, 500, DISP_FX_FADE);
			display_number(p_time->minutes, 0);
			k_msleep(1000);
			display_fade_next(DISP_FX_DIR_OUT, 250, DISP_FX_FADE);
			display_clear();

			/* Go to sleep right away */
			i = SLEEP_TIMEOUT;
		} else {
			/* Any other wakeup source will display in BCD format */
			display_bcd(p_time->hours, p_time->minutes, p_time->seconds, 0);
			clock_thread_sync();
		}
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

		stopwatch_time_t *p_time = stopwatch_time_get(0);
		if (p_time->hours) {
			display_bcd(p_time->hours,
				    p_time->minutes,
				    p_time->seconds,
				    0);
		} else {
			display_bcd(p_time->minutes,
				    p_time->seconds,
				    p_time->cents,
				    0);
		}

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

void screen_countdown(void)
{
	uint32_t i = 0;
	/* Default timer of 10 minutes  */
	static time_struct_t init_time = {
		0,
		10,
		0
	};
	time_struct_t rem_time = {0, 0, 0};

	display_clear();
	display_mono_set_color(128, 29, 214); /* Purple-ish */
	display_string("timer", 0, SCROLL_SPEED);
	k_msleep(DISP_DELAY);

	while(!state.exit_signal && !state.main)
	{
		i++;
		if(i >= SLEEP_TIMEOUT)
		{
			i = 0;
			board_suspend();
		}

		if (state.but_ur == 1) {
			/* Start */
			state.but_ur = 0;
			cd_timer_start(&init_time);
			display_string("start", 0, SCROLL_SPEED);
		}

		else if (state.but_ur == 2) {
			/* Stop */
			state.but_ur = 0;
			cd_timer_stop();
			display_string("stop", 0, SCROLL_SPEED);
		}

		if (state.but_lr) {
			state.but_lr = 0;
			if(cd_timer_state_get() == CD_TIMER_STOPPED)
			{
				/* Select countdown time */
				if(!set_time(&init_time)) {
					/* Start timer */
					cd_timer_start(&init_time);
					state_clear();
					display_string("start", 0, SCROLL_SPEED);
				}
			}
		}

		cd_timer_remaining_get(&rem_time);
		display_bcd(rem_time.hours,
			    rem_time.minutes,
			    rem_time.seconds,
			    0);

		/* Wait for either next timer tick or button
		 * interrupt */
		k_sleep(K_SECONDS(1));
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
