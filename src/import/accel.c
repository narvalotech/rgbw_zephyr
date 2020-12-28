#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/sensor.h>
#include "disp.h"
#include "accel.h"

static const struct device *sensor;

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
			accel[i] = val[i].val1 * 1000 + (val[i].val2 * 0.001);
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
