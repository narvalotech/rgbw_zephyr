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

#define VDD_CTL_NODE DT_NODELABEL(eio0)
#define VDD_CTL	     DT_GPIO_PIN(VDD_CTL_NODE, gpios)

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

static void fetch_and_display(const struct device *sensor)
{
	static unsigned int count;
	struct sensor_value accel[3];
	const char *overrun = "";
	int rc = sensor_sample_fetch(sensor);

	++count;
	if (rc == -EBADMSG) {
		/* Sample overrun.  Ignore in polled mode. */
		if (IS_ENABLED(CONFIG_LIS2DH_TRIGGER)) {
			overrun = "[OVERRUN] ";
		}
		rc = 0;
	}
	if (rc == 0) {
		rc = sensor_channel_get(sensor,
					SENSOR_CHAN_ACCEL_XYZ,
					accel);
	}
	if (rc < 0) {
		printk("ERROR: Update failed: %d\n", rc);
	} else {
		printk("#%u @ %u ms: %sx %d , y %d , z %d\n",
		       count, k_uptime_get_32(), overrun,
		       accel[0].val1,
		       accel[1].val1,
		       accel[2].val1);
	}
}

void main(void)
{
	const struct device *dev;
	int ret;

	/* Get binding to gpio port */
	/* Way too much boilerplate IMO */
	dev = device_get_binding(DT_GPIO_LABEL(VDD_CTL_NODE, gpios));
	ret = gpio_pin_configure(dev, VDD_CTL, GPIO_OUTPUT_ACTIVE | DT_GPIO_FLAGS(VDD_CTL_NODE, gpios));

	gpio_pin_set(dev, VDD_CTL, 1);
	k_msleep(100);

	rgb_led_init(&led_cfg);
	display_init();
	display_mono_set_color(0, 255, 0);

	display_clear();
	display_animate_slide(0, 50*16);

	display_clear();
	display_mono_set_color(255, 0, 0);
	display_string("hello world", 0, 50);

	gpio_pin_set(dev, VDD_CTL, 0);

	/* Test accelerometer */
	/* Need to use pinmux interface to enable sensor VDD because drivers are
	 * initialized at kernel boot time. */
	const struct device *sensor = device_get_binding(DT_LABEL(DT_NODELABEL(accel)));
	if (sensor == NULL) {
		printk("Could not get %s device\n",
		       DT_LABEL(DT_NODELABEL(accel)));
		return;
	}

	printk("Polling at 10 Hz\n");
	while (true) {
		fetch_and_display(sensor);
		k_msleep(1000);
	}

	while(1)
	{
		/* Allow deferred logging */
		k_msleep(1000);
	}
}
