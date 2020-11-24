/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

#define VDD_CTL_NODE DT_NODELABEL(eio0)
#define VDD_CTL	      DT_GPIO_PIN  (VDD_CTL_NODE, gpios)

void main(void)
{
	const struct device *dev;
	int ret;

	/* Get binding to gpio port */
	/* Way too much boilerplate IMO */
	dev = device_get_binding(DT_GPIO_LABEL(VDD_CTL_NODE, gpios));
	/* Config pin */
	ret = gpio_pin_configure(dev, VDD_CTL, GPIO_OUTPUT_ACTIVE | DT_GPIO_FLAGS(VDD_CTL_NODE, gpios));

	while (1) {
		gpio_pin_set(dev, VDD_CTL, 1);
		k_msleep(SLEEP_TIME_MS);
	}
}
