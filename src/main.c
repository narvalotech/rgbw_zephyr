/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include "rgb_led.h"
#include "disp.h"

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

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

void main(void)
{
	const struct device *dev;
	int ret;

	/* Get binding to gpio port */
	/* Way too much boilerplate IMO */
	dev = device_get_binding(DT_GPIO_LABEL(VDD_CTL_NODE, gpios));
	/* Config pin */
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

	gpio_pin_set(dev, VDD_CTL, 1);
	while(1)
	{
		/* Allow deferred logging */
		k_msleep(1000);
	}
}
