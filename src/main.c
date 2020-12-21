/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <string.h>
#include "rgb_led.h"
#include "disp.h"
#include "clock.h"
#include "board.h"
#include "accel.h"
#include "state.h"
#include "screen.h"

/* Has to be led_num + 2 long to accomodate
    * start and end words */
rgb_led_value_t led_data[30];
rgb_led_string_config_t led_cfg =
{
	.p_led_data = led_data,
	.led_num = 24,
	.brightness = 1,
	.pin_data = 0,
	.pin_clock = 0,
};

struct g_state state;

void main(void)
{
	/* Reset global state */
	memset(&state, 0, sizeof(state));

	/* Init clock module */
	clock_time_init();
	time_struct_t new_time = {12, 0, 0};
	clock_set_time(new_time);

	/* Init accelerometer lib */
	accel_init();

	board_gpio_setup();
	enable_5v(1);

	rgb_led_init(&led_cfg);
	display_init();
	display_mono_set_color(0, 255, 0);

	display_clear();
	display_animate_slide(0, 50*16);

	display_clear();
	display_mono_set_color(255, 0, 0);
	display_string("hello world", 0, 50);

	screen_time_set();
	if(state.main)
	{
		/* Test accelerometer */
		display_mono_set_color(86, 213, 245);
		accel_test_tilt();
	}
	display_mono_set_color(255, 160, 0);
	screen_clock_bcd();

	enable_5v(0);
	return;
}
