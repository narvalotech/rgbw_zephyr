#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include "state.h"

#define VDD_CTL_NODE DT_NODELABEL(eio0)
#define VDD_CTL	     DT_GPIO_PIN(VDD_CTL_NODE, gpios)
#define SW_0_PIN     DT_GPIO_PIN(DT_ALIAS(sw0), gpios) /* Lower left */
#define SW_1_PIN     DT_GPIO_PIN(DT_ALIAS(sw1), gpios) /* Lower right */
#define SW_2_PIN     DT_GPIO_PIN(DT_ALIAS(sw2), gpios) /* Upper right */

extern struct g_state state;

void button_callback(const struct device *dev, struct gpio_callback *cb,
		     uint32_t pins)
{
	if (pins & (1 << SW_0_PIN))
	{
		state.select = 1;
	}
	if (pins & (1 << SW_1_PIN))
	{
		state.exit_signal = 1;
		state.main = 1;
	}
	if (pins & (1 << SW_2_PIN))
	{
		state.exit_signal = 1;
		state.abort_disp = 1;
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
				     GPIO_INT_EDGE_TO_ACTIVE);

	gpio_pin_configure(button,
			   SW_1_PIN,
			   DT_GPIO_FLAGS(DT_ALIAS(sw1), gpios));

	gpio_pin_interrupt_configure(button,
				     SW_1_PIN,
				     GPIO_INT_EDGE_TO_ACTIVE);

	gpio_pin_configure(button,
			   SW_2_PIN,
			   DT_GPIO_FLAGS(DT_ALIAS(sw2), gpios));

	gpio_pin_interrupt_configure(button,
				     SW_2_PIN,
				     GPIO_INT_EDGE_TO_ACTIVE);

	gpio_init_callback(&button_cb_data,
			   button_callback,
			   BIT(SW_0_PIN) | BIT(SW_1_PIN) | BIT(SW_2_PIN));

	gpio_add_callback(button, &button_cb_data);
}

void board_gpio_setup(void)
{
	setup_buttons();
}

void enable_5v(bool enable)
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
