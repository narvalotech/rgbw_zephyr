#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/sensor.h>
#include <drivers/gpio.h>
#include <drivers/i2c.h>
#include "state.h"
#include "disp.h"
#include "accel.h"

static const struct device* sensor;
static const struct device* sensor_bus;
#define SENSOR_ADDR (uint8_t)DT_PROP(DT_NODELABEL(accel), reg)

extern struct g_state state;

void acc_app_init(void);

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

int accel_high_latency(bool high)
{
	struct sensor_value freq;
	if(high) {
		freq.val1 = 1;
	} else {
		freq.val1 = 50;
	}

	return sensor_attr_set(sensor,
			       SENSOR_CHAN_ACCEL_XYZ,
			       SENSOR_ATTR_SAMPLING_FREQUENCY,
			       &freq);
}

int accel_init(void)
{
	/* Need to use pinmux interface to enable sensor VDD because drivers are
	 * initialized at kernel boot time.
	 */
	sensor = device_get_binding(DT_LABEL(DT_NODELABEL(accel)));
	sensor_bus = device_get_binding(DT_LABEL(DT_BUS(DT_NODELABEL(accel))));

	if (sensor == NULL) {
		printk("Could not get %s device\n",
		       DT_LABEL(DT_NODELABEL(accel)));
		return -1;
	}

	acc_app_init();

	return 0;
}

/* Imported from ledwatch project
 * TODO: Improve upstream driver someday */
static uint8_t acc_read_reg(uint8_t reg_addr)
{
	uint8_t value = 0;

	i2c_reg_read_byte(sensor_bus,
			  SENSOR_ADDR,
			  reg_addr, &value);

	return value;
}

static void acc_write_reg(uint8_t reg_addr, uint8_t value)
{
	i2c_reg_write_byte(sensor_bus,
			   SENSOR_ADDR,
			   reg_addr, value);
}

static void acc_hpf_config(uint8_t config)
{
	acc_write_reg(0x21, config); // Write ctrl_reg2
}

static void acc_int1_sources(uint8_t sources)
{
	sources &= 0xFF; // 8 bits

	acc_write_reg(0x22, sources); // Write register
}

static void acc_int2_sources(uint8_t sources)
{
	sources &= 0xFF; // 8 bits

	acc_write_reg(0x25, sources); // Write register
}

static void acc_click_set(uint8_t sources, uint16_t threshold,
			  uint16_t limit_ms, uint16_t latency_ms,
			  uint16_t window_ms)
{
	//----Config----
	uint8_t oldcfg = acc_read_reg(0x38);
	sources &= 0x3F; // 6 bits
	sources |= oldcfg;
	acc_write_reg(0x38, sources); // Write tap_cfg


	//----Threshold----
	uint8_t fs = acc_read_reg(0x23);
	fs >>= 4;   // Data in MSB
	fs &= 0x03; // 2 bits

	switch(fs) {
		case ACC_FS_2G:
			threshold /= 16;
			break;
		case ACC_FS_4G:
		threshold /= 31;
			break;
		case ACC_FS_8G:
			threshold /= 63;
			break;
		case ACC_FS_16G:
			threshold /= 125;
			break;
		default:
			return;
	}

	threshold &= 0x7F; // 7 bits
	acc_write_reg(0x3A, threshold); // Write tap_thr


	//----Time parameters----
	float factor_ms;
	uint8_t rate = acc_read_reg(0x20);  // Read cfg_reg_1
	rate >>= 4;   // Data is in MSB
	rate &= 0x0F; // 4 bits

	switch(rate) {
		case ACC_RATE_1:
			factor_ms = 1000;
			break;
		case ACC_RATE_10:
			factor_ms = 100;
			break;
		case ACC_RATE_25:
			factor_ms = 40;
			break;
		case ACC_RATE_50:
			factor_ms = 20;
			break;
		case ACC_RATE_100:
			factor_ms = 10;
			break;
		case ACC_RATE_200:
			factor_ms = 5;
			break;
		case ACC_RATE_400:
			factor_ms = 2.5;
			break;
		case ACC_RATE_1250:
			factor_ms = 0.8;
			break;
		case ACC_RATE_1600_LP:
			factor_ms = 0.625;
			break;
		// case ACC_RATE_5000_LP:
		// 	factor_ms = 5;
		// 	break;
		default:
			return;
			break;
	}
	limit_ms /= factor_ms;
	latency_ms /= factor_ms;
	window_ms /= factor_ms;

	limit_ms &= 0x7F; // 7 bits
	latency_ms &= 0xFF; // 8 bits
	window_ms &= 0xFF; // 8 bits

	acc_write_reg(0x3B, limit_ms); // Write tap_limit
	acc_write_reg(0x3C, latency_ms); // Write tap_latency
	acc_write_reg(0x3D, window_ms); // Write tap_limit
}

/* App-level */
static void acc_enable_click(void)
{
	acc_click_set(TAP_ZD, // Enable Z double-tap, X and Y single-tap
		      800, // Tap-threshold is 0.3 G
		      200, // Tap detected if acc_value decreases within 180ms
		      80, // Pause 80ms before starting double-tap detection
		      100); // User has to tap again withing 100 ms to register a double-tap
	// Enable only Z-axis double-click
	acc_write_reg(0x38, TAP_ZD);
}

static struct gpio_callback acc_cb_data;
static void acc_callback(const struct device *dev,
			 struct gpio_callback *cb,
			 uint32_t pins)
{
	if(state.motion_wake)
		k_wakeup(state.main_tid); /* Wake from sleep */
}

#define ACC_IRQ1_PIN     DT_GPIO_PIN(DT_NODELABEL(acc_int1), gpios)
#define ACC_IRQ2_PIN     DT_GPIO_PIN(DT_NODELABEL(acc_int2), gpios)

void acc_app_init(void)
{
	/* Enable accelerometer motion app wakeup */
	state.motion_wake = 1;

	// Enable hpf on click only, fc = 1Hz @ fs=50Hz
	acc_hpf_config(HP_MODE_NORMAL | HPCLICK);
	acc_enable_click(); // Enable doubleclick detection on Z-axis

	/* Configure interrupt pins */
	const struct device *gpio_dev;
	gpio_dev = device_get_binding(DT_GPIO_LABEL(DT_NODELABEL(acc_int1), gpios));

	gpio_pin_interrupt_configure(gpio_dev,
				     ACC_IRQ1_PIN,
				     GPIO_INT_EDGE_RISING);
	gpio_pin_interrupt_configure(gpio_dev,
				     ACC_IRQ2_PIN,
				     GPIO_INT_EDGE_RISING);
	gpio_pin_configure(gpio_dev,
			   ACC_IRQ1_PIN,
			   DT_GPIO_FLAGS(DT_NODELABEL(acc_int1), gpios));
	gpio_pin_configure(gpio_dev,
			   ACC_IRQ2_PIN,
			   DT_GPIO_FLAGS(DT_NODELABEL(acc_int2), gpios));

	gpio_init_callback(&acc_cb_data,
			   acc_callback,
			   BIT(ACC_IRQ1_PIN) | BIT(ACC_IRQ2_PIN));
	gpio_add_callback(gpio_dev, &acc_cb_data);

	// Tap-detection interrupt on output 1
	acc_int1_sources(I1_CLICK);
	// Orientation detection interrupt on output 2
	acc_int2_sources(I2_CLICK | I2_IG1 | I2_HLACTIVE);
}
