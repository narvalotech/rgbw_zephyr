#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/sensor.h>
#include "disp.h"
#include "accel.h"

static const struct device *sensor;

void accel_test_tilt(void)
{
	#define TEST_TILT_THRESHOLD 300

	uint8_t display[4] = {0};
	int32_t accel[3];

	for(int i=0; i<1000; i++) {
		accel_get_mg(accel);

		if(accel[0] < -TEST_TILT_THRESHOLD) {
			display[0] = 0b01100000;
			display[1] = 0b01100000;
			display[2] = 0b01100000;
		}
		else if(accel[0] > TEST_TILT_THRESHOLD) {
			display[0] = 0b00000110;
			display[1] = 0b00000110;
			display[2] = 0b00000110;
		}
		else {
			display[0] = 0b00011000;
			display[1] = 0b00011000;
			display[2] = 0b00011000;
		}

		if(accel[1] > TEST_TILT_THRESHOLD) {
			display[1] = 0;
			display[2] = 0;
		}
		else if(accel[1] < -TEST_TILT_THRESHOLD) {
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

int accel_get_mg(int32_t accel[3])
{
	struct sensor_value val[3];

	int rc = sensor_sample_fetch(sensor);
	if (rc == 0) {
		rc = sensor_channel_get(sensor,
					SENSOR_CHAN_ACCEL_XYZ,
					val);
		for(int i=0; i<3; i++)
		{
			accel[i] = val[i].val1 * 100 + (val[i].val2 * 0.001);
		}
	}
	return rc;
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
