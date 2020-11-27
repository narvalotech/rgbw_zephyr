#include <zephyr.h>
#include <device.h>
#include <init.h>
#include <devicetree.h>
#include <drivers/gpio.h>

#define SENS_CTL_NODE   DT_NODELABEL(sens_en)
#define SENS_CTL	DT_GPIO_PIN(SENS_CTL_NODE, gpios)

#define PIN_SENS_EN    NRF_GPIO_PIN_MAP(1, 9)

static int pinmux_core840_init(const struct device *port)
{
	ARG_UNUSED(port);

	const struct device *dev;
	int ret;

	dev = device_get_binding(DT_GPIO_LABEL(SENS_CTL_NODE, gpios));
	ret = gpio_pin_configure(dev, SENS_CTL, GPIO_OUTPUT_ACTIVE | DT_GPIO_FLAGS(SENS_CTL_NODE, gpios));

	gpio_pin_set(dev, SENS_CTL, 1);

	return 0;
}

SYS_INIT(pinmux_core840_init, PRE_KERNEL_1, 0);
