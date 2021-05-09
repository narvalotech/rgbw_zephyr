#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include "board.h"
#include "state.h"
#include "disp.h"
#include "clock.h"
#include "accel.h"
#include "motor.h"

#define VDD_CTL_NODE DT_NODELABEL(eio0)
#define VDD_CTL	     DT_GPIO_PIN(VDD_CTL_NODE, gpios)
#define SW_0_PIN     DT_GPIO_PIN(DT_ALIAS(sw0), gpios) /* Lower left */
#define SW_1_PIN     DT_GPIO_PIN(DT_ALIAS(sw1), gpios) /* Lower right */
#define SW_2_PIN     DT_GPIO_PIN(DT_ALIAS(sw2), gpios) /* Upper right */
#define BATT_NODE    DT_NODELABEL(batmon_en)
#define BATT_PIN     DT_GPIO_PIN(BATT_NODE, gpios)
#define MOTOR_NODE   DT_NODELABEL(hapt_gpio)
#define MOTOR_PIN    DT_GPIO_PIN(MOTOR_NODE, gpios)

extern struct g_state state;

void board_suspend(void)
{
	/* Kill display power */
	display_clear();
	board_enable_5v(0);

	/* Reduce sensor power */
	/* Cannot do this if motion wakeup is enabled
	 * since HP filter is calibrated for 50Hz
	 * TODO: make this configurable */
	/* accel_high_latency(1); */

	/* Put clock counter into high-latency mode */
	clock_set_high_latency(1);

	/* Put CPU to sleep until interrupted */
	k_sleep(K_FOREVER);

	/* Restore sensor power */
	/* accel_high_latency(0); */

	/* Put clock counter into low-latency mode */
	clock_set_high_latency(0);

	/* Restore display power */
	board_enable_5v(1);
	display_clear();
}

static void button_timer_callback(struct k_timer *timer_id)
{
	state.but_long_press = 1;
}
K_TIMER_DEFINE(button_timer, button_timer_callback, NULL);

static void button_callback(const struct device *dev, struct gpio_callback *cb,
			    uint32_t pins)
{
	gpio_port_value_t port_val = 0;

	/* All pushbuttons are on the same port */
	dev = device_get_binding(DT_GPIO_LABEL(DT_ALIAS(sw0), gpios));
	gpio_port_get(dev, &port_val);

	if(port_val == 0) {
		/* Abort running timer */
		k_timer_stop(&button_timer);
	}
	else {
		/* One-shot timer */
		k_timer_start(&button_timer, K_SECONDS(1), K_NO_WAIT);
		return;
	}

	k_wakeup(state.main_tid); /* Wake from sleep */

	if (pins & (1 << SW_0_PIN))
	{
		if(state.but_long_press) {
			state.but_long_press = 0;
			state.but_ll = 2;
			/* Go to main screen immediately */
			state.main = 1;
			state.abort_disp = 1;  /* Aborts scrolling text */
			state.exit_signal = 1; /* Exits current activity */
			clock_thread_unblock();
		}
		else {
			state.but_ll = 1;
			state.abort_disp = 1;  /* Aborts scrolling text */
			state.exit_signal = 1; /* Exits current activity */
			clock_thread_unblock();
		}
	}
	if (pins & (1 << SW_1_PIN))
	{
		if(state.but_long_press) {
			state.but_long_press = 0;
			state.but_lr = 2;
		}
		else {
			state.but_lr = 1;
		}
	}
	if (pins & (1 << SW_2_PIN))
	{
		if(state.but_long_press) {
			state.but_long_press = 0;
			state.but_ur = 2;
		}
		else {
			state.but_ur = 1;
			state.select = 1;
			state.abort_disp = 1;
		}
	}
}

static struct gpio_callback button_cb_data;
static void setup_buttons(void)
{
	const struct device *button;
	button = device_get_binding(DT_GPIO_LABEL(DT_ALIAS(sw0), gpios));

	gpio_pin_configure(button,
			   SW_0_PIN,
			   DT_GPIO_FLAGS(DT_ALIAS(sw0), gpios));

	gpio_pin_interrupt_configure(button,
				     SW_0_PIN,
				     GPIO_INT_EDGE_BOTH);

	gpio_pin_configure(button,
			   SW_1_PIN,
			   DT_GPIO_FLAGS(DT_ALIAS(sw1), gpios));

	gpio_pin_interrupt_configure(button,
				     SW_1_PIN,
				     GPIO_INT_EDGE_BOTH);

	gpio_pin_configure(button,
			   SW_2_PIN,
			   DT_GPIO_FLAGS(DT_ALIAS(sw2), gpios));

	gpio_pin_interrupt_configure(button,
				     SW_2_PIN,
				     GPIO_INT_EDGE_BOTH);

	gpio_init_callback(&button_cb_data,
			   button_callback,
			   BIT(SW_0_PIN) | BIT(SW_1_PIN) | BIT(SW_2_PIN));

	gpio_add_callback(button, &button_cb_data);
}

static void setup_batt(void)
{
	const struct device *batt;

	batt = device_get_binding(DT_GPIO_LABEL(BATT_NODE, gpios));
	gpio_pin_configure(batt, BATT_PIN,
			   GPIO_OUTPUT | DT_GPIO_FLAGS(BATT_NODE, gpios));
	gpio_pin_set(batt, BATT_PIN, 0);
}

static void setup_motor(void)
{
	const struct device *motor;

	motor = device_get_binding(DT_GPIO_LABEL(MOTOR_NODE, gpios));
	gpio_pin_configure(motor, MOTOR_PIN,
			   GPIO_OUTPUT | DT_GPIO_FLAGS(MOTOR_NODE, gpios));
	gpio_pin_set(motor, MOTOR_PIN, 0);

	/* Setup PWM after setting up GPIO fallback */
	motor_init();
}

void board_gpio_setup(void)
{
	setup_buttons();
	setup_batt();
	setup_motor();
}

void board_enable_5v(bool enable)
{
	const struct device *dev;

	dev = device_get_binding(DT_GPIO_LABEL(VDD_CTL_NODE, gpios));

	gpio_pin_configure(dev, VDD_CTL, GPIO_OUTPUT | DT_GPIO_FLAGS(VDD_CTL_NODE, gpios));

	if(enable)
	{
		gpio_pin_set(dev, VDD_CTL, 1);
		/* Wait for 5V ramp-up */
		k_msleep(2);
	}
	else
	{
		gpio_pin_set(dev, VDD_CTL, 0);
	}
}

#define SENS_CTL_NODE   DT_NODELABEL(sens_en)
#define SENS_CTL	DT_GPIO_PIN(SENS_CTL_NODE, gpios)
void board_enable_vdd_ext(bool enable)
{
	const struct device *dev;
	int ret;

	dev = device_get_binding(DT_GPIO_LABEL(SENS_CTL_NODE, gpios));
	ret = gpio_pin_configure(dev, SENS_CTL, GPIO_OUTPUT_ACTIVE | DT_GPIO_FLAGS(SENS_CTL_NODE, gpios));

	gpio_pin_set(dev, SENS_CTL, enable);
}
