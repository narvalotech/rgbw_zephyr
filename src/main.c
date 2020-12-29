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
	/* Store main thread ID */
	state.main_tid = k_current_get();

	/* Init clock module */
	clock_time_init();
	time_struct_t new_time = {12, 0, 0};
	clock_set_time(new_time);

	/* Init accelerometer lib */
	accel_init();

	board_gpio_setup();
	board_enable_5v(1);

	rgb_led_init(&led_cfg);
	display_init();
	display_mono_set_color(0, 255, 0);

	display_clear();
	display_animate_slide(0, 50*16);

	display_clear();
	display_mono_set_color(255, 0, 0);
	display_string("hello", 0, 50);

	main_state_set(PGM_STATE_CLOCK_BCD);
	while(state.pgm_state != PGM_STATE_HIBERNATE)
	{
		main_state_loop();
	}

	board_enable_5v(0);
	return;
}
