#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/sensor.h>
#include "disp.h"
#include "accel.h"

static const struct device *sensor;

void accel_test_tilt(void)
{
	uint8_t display[4] = {0};
	int32_t acc_x = 0, acc_y = 0;
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

int accel_init(void)
{
	/* Need to use pinmux interface to enable sensor VDD because drivers are
	 * initialized at kernel boot time.
	 */
	sensor = device_get_binding(DT_LABEL(DT_NODELABEL(accel)));

	if (sensor == NULL) {
		printk("Could not get %s device\n",
		       DT_LABEL(DT_NODELABEL(accel)));
		return -1;
	}

	return 0;
}
