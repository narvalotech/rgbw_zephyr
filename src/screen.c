#include <zephyr.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include "img_mgmt/img_mgmt.h"
#include "screen.h"
#include "clock.h"
#include "disp.h"
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
#include "alarm.h"

extern struct g_state state;

#define SCROLL_SPEED 50
#define DISP_DELAY 200
#define SLEEP_TIMEOUT 10

/* numberSelector defines */
#define ITEM_SCROLL_THR 2000
#define TILT_ANGLE_COMPENSATION (+3000)
#define ACCEL_INVERTED (-1)

static int abs(int val)
{
	if(val < 0)
	{
		val = 0 - val;
	}

	return val;
}

/* TODO: clean this up, remove int16's etc */
uint32_t numberSelector(uint16_t defaultNum,
			uint16_t startNum,
			uint16_t endNum,
			uint8_t displayType)
{
	uint32_t currentNumber = defaultNum;
	int32_t accel;
	uint32_t dispTime = 1000;
	int32_t acc_val[3] = {0};

	state.but_ur = 0;
	state.abort = 0;

	while(!state.abort)
	{
		// Display current number
		switch(displayType)
		{
			case DISPLAY_DIGITAL:
			display_number(currentNumber, dispTime);
			break;
			case DISPLAY_BCD:
			display_bcd(0, currentNumber, 0, dispTime);
			break;
			default:
			display_bcd(0, currentNumber, 0, dispTime);
			break;
		}

		// Get acceleration data
		accel_get_mg(acc_val);
		accel = acc_val[1] * ACCEL_INVERTED;
		accel += TILT_ANGLE_COMPENSATION;

		// Display time is proportional to tilt angle
		dispTime = abs(accel) / 7;
		if(dispTime > 900)
			dispTime = 900;	/* Take care of overflow */
		dispTime = 1000 - dispTime;

		if(state.but_ur)
		{
			return currentNumber;
		}

		// Increment/decrement number
		if(accel > ITEM_SCROLL_THR)
		{
			if(currentNumber < endNum)
				currentNumber++;
			else
				currentNumber = startNum;
		}
		else if(accel < -ITEM_SCROLL_THR)
		{
			if(currentNumber > startNum)
				currentNumber--;
			else
				currentNumber = endNum;
		}
	}

	return currentNumber;
}

static int input_date(struct date_time* p_date)
{
	struct date_time date;
	memset(&date, 0, sizeof(date));

	display_clear();
	state_clear();
	display_string("set date",0,SCROLL_SPEED);
	k_msleep(DISP_DELAY);
	if(state.next || state.main)
	{
		return -1;
	}

	display_string("  year", 0, SCROLL_SPEED);
	k_msleep(DISP_DELAY);
	date.year = 2000 + numberSelector(p_date->year, 0, 99, DISPLAY_DIGITAL);
	if(state.next || state.main)
	{
		return -1;
	}
	k_msleep(DISP_DELAY);

	display_string("  month", 0, SCROLL_SPEED);
	k_msleep(DISP_DELAY);
	date.month = numberSelector(p_date->month, 1, 12, DISPLAY_DIGITAL);
	if(state.next || state.main)
	{
		return -1;
	}
	k_msleep(DISP_DELAY);

	display_string("  day", 0, SCROLL_SPEED);
	k_msleep(DISP_DELAY);
	date.day = numberSelector(p_date->day, 1, 31, DISPLAY_DIGITAL);
	if(state.next || state.main)
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

static int input_time(time_struct_t* p_time, bool seconds)
{
	time_struct_t time = {0, 0, 0};

	display_clear();
	state_clear();
	display_string("set time",0,SCROLL_SPEED);
	k_msleep(DISP_DELAY);
	if(state.next || state.main) {
		return -1;
	}

	display_string("  hours",0,SCROLL_SPEED);
	k_msleep(DISP_DELAY);
	time.hours = (uint8_t)numberSelector(p_time->hours, 0, 23, DISPLAY_DIGITAL);
	if(state.next || state.main) {
		return -1;
	}
	k_msleep(DISP_DELAY);

	display_string("  minutes",0,SCROLL_SPEED);
	k_msleep(DISP_DELAY);
	time.minutes = (uint8_t)numberSelector(p_time->minutes, 0, 59, DISPLAY_DIGITAL);
	if(state.next || state.main) {
		return -1;
	}
	k_msleep(DISP_DELAY);

	if(seconds) {
		display_string("  seconds",0,SCROLL_SPEED);
		k_msleep(DISP_DELAY);
		time.seconds = (uint8_t)numberSelector(p_time->seconds, 0, 59, DISPLAY_DIGITAL);
		if(state.next || state.main) {
			return -1;
		}
		k_msleep(DISP_DELAY);
	}

	/* If we were not aborted during whole selection process
	 * then save selection. */
	p_time->hours = time.hours;
	p_time->minutes = time.minutes;
	p_time->seconds = time.seconds;

	return 0;
}

static int input_days(uint8_t* p_days)
{
	uint8_t days = *p_days;

	display_clear();
	state_clear();
	display_string("set days", 0, SCROLL_SPEED);
	k_msleep(DISP_DELAY);
	if(state.next || state.main)
	{
		return -1;
	}

	/* Loop through each day in the week, enabling or disabling it. */
	/* Uses day_of_week_t values. */
	/* Week is read from right to left. */
	for(uint8_t day = MONDAY; day <= SUNDAY && !state.main; day++)
	{
		/* Keep previous enable state */
		uint8_t en_day = (days & (1 << day)) >> day;

		state.but_ur = 0;
		state.but_lr = 0;

		/* UR moves to next day, LR enables/disables current day */
		while(!state.but_ur && !state.main)
		{
			display_bytes(1 << day,
				      days,
				      0,
				      0);
			k_sleep(K_FOREVER); /* Wait for next button press */
			if(state.but_lr) {
				en_day ^= 1;
				state.but_lr = 0;
				days &= ~(1 << day);
				days |= (en_day & 0x01) << day;
			}
		}
	}

	state_clear();
	if(state.main)
	{
		return -1;
	}

	/* Save value if user did not abort */
	*p_days = days;

	return 0;
}

void screen_time_set(void)
{
	time_struct_t newTime;

	/* Start selector at current time */
	memcpy(&newTime, clock_get_time_p(), sizeof(newTime));

	display_mono_set_color(255, 0, 0);
	if(!input_time(&newTime, false))
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
	while(!state.next)
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
				display_clear();

				/* Start with the day */
				display_number(cal_get_day(), 0);
				k_msleep(500);

				/* And scroll the rest of the date */
				sprintf(date_buf, "  %s %d  ",
					cal_month_string[cal_get_month() - 1],
					cal_get_year());
				display_string(date_buf, 0, 80);

				if(state.but_ur == 2) {
					/* If user long-pressed button during
					 * date display, then set the date. */
					struct date_time date;
					if(input_date(&date) == 0) {
						cal_set_date(&date);
					}
				}
				/* Clear button state just in case */
				state.but_ur = 0;

				/* Go to sleep right away */
				i = SLEEP_TIMEOUT;
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
		display_clear();
		display_animate_slide(0, 50*16);
		/* Leave more energy for flash swap to succeed */
		board_enable_5v(0);
		NVIC_SystemReset();
	}

	if(state.but_ll == 2) {
		/* Toggle wake-on-movement */
		/* Very useful for not draining the battery when biking */
		display_clear();
		state.motion_wake ^= 1;
		if(state.motion_wake)
			display_string("motion on", 0, SCROLL_SPEED);
		else
			display_string("motion off", 0, SCROLL_SPEED);
	}

	state_clear();
}

void screen_stopwatch(void)
{
	uint32_t i = 0;

	display_clear();
	display_mono_set_color(0, 255, 0);
	display_string("stw", 0, SCROLL_SPEED);

	while(!state.next && !state.main)
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

	while(!state.next && !state.main)
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
				if(!input_time(&init_time, false)) {
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

void screen_alarm_view(void)
{
	/* View and configure alarm */
	alarm_struct_t alarm_time;

	display_clear();
	display_mono_set_color(251, 255, 20); /* Yellow */
	display_string("alarm", 0, SCROLL_SPEED);

	while(!state.next && !state.main) {
		/* Enable/disable alarm */
		if (state.but_ur == 2) {
			state.but_ur = 0;
			/* Toggle state */
			alarm_enable(!alarm_is_enabled());
			if(alarm_is_enabled())
				display_string("enabled", 0, SCROLL_SPEED);
			else
				display_string("disabled", 0, SCROLL_SPEED);
		}

		/* Set alarm */
		if (state.but_lr == 2) {
			state.but_lr = 0;
			/* Start from the previous settings */
			alarm_get(&alarm_time);
			if(input_time(&alarm_time.time, false))
				continue;
			if(input_days(&alarm_time.days))
				continue;
			/* Apply the new settings */
			alarm_set(&alarm_time);
		}

		alarm_get(&alarm_time);
		/* Display:
		 * - Top: configured days right to left, MSb is enabled/disabled
		 * - Mid: Hours BCD
		 * - Bot: Minutes BCD */
		display_bytes(alarm_time.days | ((alarm_is_enabled() & 1)),
			      ((alarm_time.time.hours / 10) << 4) +
				      (alarm_time.time.hours % 10),
			      ((alarm_time.time.minutes / 10) << 4) +
				      (alarm_time.time.minutes % 10), 0);

		/* Show settings for 10s, then sleep if user did not press
		 * any buttons */
		k_msleep(10000);
		if (!state_is_button_pressed())
			board_suspend();
		else
			state.abort = 0;
	}

	state_clear();
	display_clear();
}

void screen_alarm_ring(void)
{
	/* Shown when alarm is due, handles snooze and stop.
	 * This screen should not be selectable by the user, should only be
	 * shown by the alarm module. */
	bool exit = false;
	time_struct_t* p_time = clock_get_time_p();

	display_clear();
	display_mono_set_color(185, 78, 2); /* Brown-ish */

	/* Start ringing the alarm */
	alarm_start();

	while(!exit)
	{
		if (state.but_ur == 1) {
			state.but_ur = 0;
			alarm_snooze(10);
			exit = true;
			display_string("snooze", 0, SCROLL_SPEED);
		} else if(state.but_lr == 1) {
			state.but_lr = 0;
			alarm_stop();
			exit = true;
			display_string("stop", 0, SCROLL_SPEED);
		}

		/* Show the time while the user wakes up */
		display_bcd(p_time->hours,
			    p_time->minutes,
			    p_time->seconds, 0);
		/* Sync on seconds */
		clock_thread_sync();
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

	while(!state.next && !state.main)
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
	while(!batt_mv && !state.next && !state.main)
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
	if(!state.next && !state.main)
		k_sleep(K_SECONDS(5));

	state_clear();
	display_clear();
}

void screen_ble(void)
{
	display_clear();
	display_mono_set_color(0, 0, 255); /* Blue */
	display_string("bluetooth", 0, SCROLL_SPEED);

	while(!state.next && !state.main)
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

void screen_dfu_end(void)
{
	state_clear();
	board_enable_5v(1);
	display_clear();
	display_mono_set_color(0, 0, 255); /* Blue */
	display_string("dfu ok", 0, SCROLL_SPEED);

	/* Confirm newly-uploaded image */
	img_mgmt_state_set_pending(1, 1);

	/* Reset to swap and boot image */
	display_clear();
	display_animate_slide(0, 50*16);
	/* Leave more energy for flash swap to succeed */
	board_enable_5v(0);
	NVIC_SystemReset();
}
