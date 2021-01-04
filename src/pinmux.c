#include <zephyr.h>
#include <device.h>
#include <init.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include "board.h"

static int pinmux_core840_init(const struct device *port)
{
	ARG_UNUSED(port);

	board_enable_vdd_ext(1);

	return 0;
}

SYS_INIT(pinmux_core840_init, PRE_KERNEL_1, 0);
