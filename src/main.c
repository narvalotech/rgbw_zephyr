/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <drivers/sensor.h>
#include "rgb_led.h"
#include "disp.h"
#include "clock.h"
#include "board.h"

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

static void test_tilt(const struct device *sensor)
{
	uint8_t display[4] = {0};
	int32_t acc_x = 0,acc_y = 0;
	struct sensor_value accel[3];
	int rc = -1;

	for(int i=0; i<1000; i++) {
		rc = sensor_sample_fetch(sensor);
		if (rc == 0) {
			rc = sensor_channel_get(sensor,
						SENSOR_CHAN_ACCEL_XYZ,
						accel);
			acc_x = accel[0].val1 * 1000 + (accel[0].val2 * 0.001);
			acc_y = accel[1].val1 * 1000 + (accel[1].val2 * 0.001);
		}

		if(acc_x < -1000) {
			display[0] = 0b01100000;
			display[1] = 0b01100000;
			display[2] = 0b01100000;
		}
		else if(acc_x > 1000) {
			display[0] = 0b00000110;
			display[1] = 0b00000110;
			display[2] = 0b00000110;
		}
		else {
			display[0] = 0b00011000;
			display[1] = 0b00011000;
			display[2] = 0b00011000;
		}

		if(acc_y > 1000) {
			display[1] = 0;
			display[2] = 0;
		}
		else if(acc_y < -1000) {
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
}

void button_callback(const struct device *dev, struct gpio_callback *cb,
		     uint32_t pins)
{
	printk("Button pressed: 0x%x", pins);
}

static void test_clock(void)
{
	/* Init clock module */
	clock_time_init();
	time_struct_t new_time = {1, 2, 3};
	clock_set_time(new_time);
	/* Updated time value pointer */
	time_struct_t* p_time = clock_get_time_p();

	/* Display time for 10 seconds */
	for(int i=0; i<10; i++)
	{
		display_bcd(p_time->hours, p_time->minutes, p_time->seconds, 0);
		clock_thread_sync();
	}
}

void main(void)
{
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

	display_mono_set_color(255, 160, 0);
	test_clock();

	/* Test accelerometer */
	/* Need to use pinmux interface to enable sensor VDD because drivers are
	 * initialized at kernel boot time. */
	const struct device *sensor = device_get_binding(DT_LABEL(DT_NODELABEL(accel)));
	if (sensor == NULL) {
		printk("Could not get %s device\n",
		       DT_LABEL(DT_NODELABEL(accel)));
		return;
	}

	display_mono_set_color(86, 213, 245);
	test_tilt(sensor);

	enable_5v(0);
	return;
}
