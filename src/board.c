#include "board.h"
#include "i2c.h"
#include "accel.h"

/**************** Accelerometer-related utility functions ****************/
void acc_enable_orientation() {
	acc_intgen_config(IG_6D_POS | I1_YL);  // Enable 'd' position detection
	acc_intgen_duration(900);              // User has to stay in position for 500ms
	acc_intgen_threshold(700);             // Set threshold to 0.9 G
}

void acc_disable_orientation() {
	acc_intgen_config(0);  // disable position detection
}

void acc_enable_hpf() {
	acc_hpf_config(HP_MODE_NORMAL | HPCLICK); // Enable hpf on click only, fc = 1Hz @ fs=50Hz
}

void acc_disable_hpf() {
	acc_hpf_config(0);
}

void acc_enable_click() {
	acc_click_set(TAP_ZD, // Enable Z double-tap, X and Y single-tap
				  800,                      // Tap-threshold is 0.3 G
				  200,                      // Tap detected if acc_value decreases within 180ms
				  80,                       // Pause 80ms before starting double-tap detection
				  100);                     // User has to tap again withing 100 ms to register
											// a double-tap
}

void acc_disable_4D() {
	acc_write_reg(0x38, TAP_ZD); // Write tap_cfg
}

void acc_enable_4D() {
	acc_write_reg(0x38, TAP_ZD | TAP_YS | TAP_XS); // Write tap_cfg
}

void acc_app_init() {
	acc_init(PIN_ACC_SA0_LVL); // Init accelerometer
	/* acc_lowpower(1);       // Set low-power mode */
	acc_enable_orientation(); // Enable orientation detection
	// acc_disable_orientation();
	acc_enable_hpf();         // Enable high pass filter on click/tap detection
	// acc_disable_hpf();
	acc_enable_click();       // Enable click detection on 6 axes
	acc_disable_4D();         // Enable only Z-axis double-click

	acc_int1_sources(I1_CLICK); // Tap-detection interrupt on output 1
	acc_int2_sources(I2_CLICK | I2_IG1 | I2_HLACTIVE);   // Orientation detection interrupt on output 2
}

/**************** Debug-related utility functions ****************/
void dbg_led_init()
{
    nrf_gpio_cfg_output(PIN_DBG_LED_0);
    nrf_gpio_cfg_output(PIN_DBG_LED_1);
    nrf_gpio_cfg_output(PIN_DBG_LED_2);
    nrf_gpio_cfg_output(PIN_DBG_LED_3);

    nrf_gpio_pin_set(PIN_DBG_LED_0);
    nrf_gpio_pin_set(PIN_DBG_LED_1);
    nrf_gpio_pin_set(PIN_DBG_LED_2);
    nrf_gpio_pin_set(PIN_DBG_LED_3);
}

void dbg_led_on(uint32_t led)
{
    nrf_gpio_pin_clear(led);
}

void dbg_led_off(uint32_t led)
{
    nrf_gpio_pin_set(led);
}
