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

#define VDD_CTL_NODE DT_NODELABEL(eio0)
#define VDD_CTL	     DT_GPIO_PIN(VDD_CTL_NODE, gpios)
#define SW_0_PIN     DT_GPIO_PIN(DT_ALIAS(sw0), gpios) /* Lower left */
#define SW_1_PIN     DT_GPIO_PIN(DT_ALIAS(sw1), gpios) /* Lower right */
#define SW_2_PIN     DT_GPIO_PIN(DT_ALIAS(sw2), gpios) /* Upper right */

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

static void enable_5v(bool enable)
{
	const struct device *dev;

	dev = device_get_binding(DT_GPIO_LABEL(VDD_CTL_NODE, gpios));

	gpio_pin_configure(dev, VDD_CTL, GPIO_OUTPUT | DT_GPIO_FLAGS(VDD_CTL_NODE, gpios));

	if(enable)
	{
		gpio_pin_set(dev, VDD_CTL, 1);
		k_msleep(100);
	}
	else
	{
		gpio_pin_set(dev, VDD_CTL, 0);
	}
}

static struct gpio_callback button_cb_data;

void button_pressed(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
}

static void setup_buttons(void)
{
	const struct device *button;
	button = device_get_binding(DT_GPIO_LABEL(DT_ALIAS(sw0), gpios));

	gpio_pin_configure(button,
			   SW_0_PIN,
			   DT_GPIO_FLAGS(DT_ALIAS(sw0), gpios));

	gpio_pin_interrupt_configure(button,
				     SW_0_PIN,
				     GPIO_INT_EDGE_TO_ACTIVE);

	gpio_init_callback(&button_cb_data,
			   button_pressed,
			   BIT(SW_0_PIN));

	gpio_add_callback(button, &button_cb_data);
}

struct k_timer my_timer;
static void my_expiry_function(struct k_timer *timer_id)
{
	clock_increment_seconds();
}

K_TIMER_DEFINE(my_timer, my_expiry_function, NULL);

static void test_clock(void)
{
	/* Init clock module */
	clock_time_init();
	time_struct_t new_time = {1, 2, 3};
	clock_set_time(new_time);
	/* Updated time value pointer */
	time_struct_t* p_time = clock_get_time_p();

	k_timer_start(&my_timer, K_SECONDS(1), K_SECONDS(1));

	/* Display time for 10 seconds */
	for(int i=0; i<10; i++)
	{
		display_bcd(p_time->hours, p_time->minutes, p_time->seconds, 0);
		k_timer_status_sync(&my_timer);
	}
}

void main(void)
{
	setup_buttons();
	enable_5v(1);

	rgb_led_init(&led_cfg);
	display_init();
	display_mono_set_color(0, 255, 0);

	display_clear();
	display_animate_slide(0, 50*16);

	display_clear();
	display_mono_set_color(255, 0, 0);
	display_string("hello world", 0, 50);

	test_clock();

	enable_5v(0);

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
}
