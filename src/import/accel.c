#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "i2c.h"
#include "accel.h"
#include "board.h"

#define ACCEL_I2C_SDA PIN_FP_SDA
#define ACCEL_I2C_SCL PIN_FP_SCL

#ifndef ACCEL_X_ORIENTATION
#define ACCEL_X_ORIENTATION 1
#endif
#ifndef ACCEL_Y_ORIENTATION
#define ACCEL_Y_ORIENTATION 1
#endif
#ifndef ACCEL_Z_ORIENTATION
#define ACCEL_Z_ORIENTATION 1
#endif

static uint8_t accel_address = 0;
static uint8_t TxData[50] = {0};
static uint8_t RxData[50] = {0};

uint8_t acc_read_reg(uint8_t reg) {
	TxData[0] = reg & 0xFF;
	i2c_write_read(accel_address, TxData, 1, RxData, 2);

	return RxData[0];
}

void acc_write_reg(uint8_t reg, uint8_t data) {
	TxData[0] = reg;
	TxData[1] = data;
	i2c_write(accel_address, TxData, 2);
}

void acc_init(bool sa0) {
	if(sa0)
		accel_address = 0x19;	/* SA0 connected to VCC */
	else
		accel_address = 0x18;	/* SA0 connected to GND */

	i2c_init(accel_address, ACCEL_I2C_SCL, ACCEL_I2C_SDA);

	TxData[0] = 0x20 | 0x80;
	TxData[1] = 0b01000111;  // Normal mode, XYZ axes, 50Hz
	i2c_write(accel_address, TxData,2);
}

void acc_read3(int16_t* x, int16_t* y, int16_t* z) {
	TxData[0] = 0x28 | 0x80; // Read address 0x28, with auto-increment (0x80)
	i2c_write_read(accel_address, TxData,1, RxData,6); // Read 6 bytes (3x 16-bit words)

	*x = RxData[0] + (RxData[1]<<8);
	*x *= ACCEL_X_ORIENTATION;

	*y = RxData[2] + (RxData[3]<<8);
	*y *= ACCEL_Y_ORIENTATION;

	*z = RxData[4] + (RxData[5]<<8);
	*z *= ACCEL_Z_ORIENTATION;
}

int acc_read_x() {
	TxData[0] = 0x28 | 0x80;
	i2c_write_read(accel_address, TxData,1, RxData,2);

	int16_t accel = RxData[0] | (RxData[1]<<8);
	accel *= ACCEL_X_ORIENTATION;

	return (int)accel;
}

int acc_read_y() {
	TxData[0] = 0x2A | 0x80;
	i2c_write_read(accel_address, TxData,1, RxData,2);

	int16_t accel = RxData[0] | (RxData[1]<<8);
	accel *= ACCEL_Y_ORIENTATION;

	return (int)accel;
}

int acc_read_z() {
	TxData[0] = 0x2C | 0x80;
	i2c_write_read(accel_address, TxData,1, RxData,2);

	int16_t accel = RxData[0] | (RxData[1]<<8);
	accel *= ACCEL_Z_ORIENTATION;

	return (int)accel;
}

void acc_lowpower(uint8_t mode) {
	// Low-Power mode : 1, Normal mode : 0
	mode &= 1;

	uint8_t config = acc_read_reg(0x20);  // Read config
	config &= ~0x80;					  // Clear mode bit
	config |= mode<<3;					// Set mode bit
	acc_write_reg(0x20, config);		  // Write config
}

void acc_rate(uint8_t rate) {
	rate &= 0x0F; // 4 bits

	uint8_t config = acc_read_reg(0x20);  // Read control_reg_1
	config &= 0x0F;					   // Clear ODR bits
	config |= rate<<4;					// Set data rate
	acc_write_reg(0x20, config);		  // Write config
}

void acc_scale(uint8_t scale) {
	scale &= 0x03; // 2 bits

	uint8_t config = acc_read_reg(0x23);  // Read control_reg_4
	config &= ~0x30;					  // Clear scale bits
	config |= scale<<4;				   // Set full-scale value
	acc_write_reg(0x20, config);		  // Write register
}

void acc_highres(uint8_t highres) {
	highres &= 1; // 1 bit, 1:enable, 0:disable

	uint8_t config = acc_read_reg(0x23);  // Read control_reg_4
	config &= ~0x08;					  // Clear bits
	config |= highres<<3;				 // Set full-scale value
	acc_write_reg(0x20, config);		  // Write register
}

void acc_intgen_config(uint8_t config) {
	config &= 0xFF; // 8 bits

	acc_write_reg(0x30, config);		  // Write register
}

void acc_intgen_duration(uint16_t ms) {
	uint8_t rate = acc_read_reg(0x20);  // Read cfg_reg_1
	rate >>= 4;   // Data is in MSB
	rate &= 0x0F; // 4 bits

	switch(rate) {
		case ACC_RATE_1:
			ms /= 1000;
			break;
		case ACC_RATE_10:
			ms /= 100;
			break;
		case ACC_RATE_25:
			ms /= 40;
			break;
		case ACC_RATE_50:
			ms /= 20;
			break;
		case ACC_RATE_100:
			ms /= 10;
			break;
		case ACC_RATE_200:
			ms /= 5;
			break;
		case ACC_RATE_400:
			ms /= 2.5;
			break;
		case ACC_RATE_1250:
			ms *= 1.25;
			break;
		case ACC_RATE_1600_LP:
			ms *= 1.6;
			break;
		// case ACC_RATE_5000_LP:
		// 	ms *= 5;
		// 	break;
		default:
			return;
			break;
	}

	ms &= 0x7F; // 7 bits
	acc_write_reg(0x33, ms); // Write int1_duration
}

void acc_intgen_threshold(uint16_t th_mg) {
	uint8_t fs = acc_read_reg(0x23);
	fs >>= 4;   // Data in MSB
	fs &= 0x03; // 2 bits

	switch(fs) {
		case ACC_FS_2G:
			th_mg /= 16;
			break;
		case ACC_FS_4G:
		th_mg /= 31;
			break;
		case ACC_FS_8G:
			th_mg /= 63;
			break;
		case ACC_FS_16G:
			th_mg /= 125;
			break;
		default:
			return;
	}

	th_mg &= 0x7F; // 7 bits
	acc_write_reg(0x32, th_mg); // Write int1_thr
}

uint8_t acc_intgen_event() {
	uint8_t intgen = acc_read_reg(0x31);  // Read int1_src register (resets int. flag)

	if((intgen & I1_INT)==0) return 0; // Check if interrupt has occured
	else return intgen;
}

void acc_int1_sources(uint8_t sources) {
	sources &= 0xFF; // 8 bits

	acc_write_reg(0x22, sources);		  // Write register
}

void acc_int2_sources(uint8_t sources) {
	sources &= 0xFF; // 8 bits

	acc_write_reg(0x25, sources);		  // Write register
}

void acc_click_set(uint8_t sources, uint16_t threshold, uint16_t limit_ms, uint16_t latency_ms, uint16_t window_ms) {
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

uint8_t acc_click_event() {
	uint8_t tap = acc_read_reg(0x39);  // Read Tap Interrupt register (resets int. flag)

	if((tap & TAP_INT)==0) return 0; // Check if interrupt has occured

	else if((tap & TAP_S & TAP_Z) != 0)
		return CLICK;
	else if((tap & TAP_D & TAP_Z) != 0)
		return DCLICK;

	else if((tap & TAP_X & TAP_SIGN) != 0)
		return RIGHT;
	else if((tap & TAP_X) != 0)
		return LEFT;

	else if((tap & TAP_Y & TAP_SIGN) != 0)
		return UP;
	else if((tap & TAP_Y) != 0)
		return DOWN;

	else return tap;
}

void acc_hpf_config(uint8_t config) {
	acc_write_reg(0x21, config); // Write ctrl_reg2
}

void acc_hpf_set_ref(uint16_t ref) {
	uint8_t fs = acc_read_reg(0x23);
	fs >>= 4;   // Data in MSB
	fs &= 0x03; // 2 bits

	switch(fs) {
		case ACC_FS_2G:
			ref /= 16;
			break;
		case ACC_FS_4G:
		ref /= 31;
			break;
		case ACC_FS_8G:
			ref /= 63;
			break;
		case ACC_FS_16G:
			ref /= 125;
			break;
		default:
			return;
	}

	ref &= 0xFF; // 8 bits
	acc_write_reg(0x26, ref); // Write reference
}

uint8_t acc_hpf_reset() {
	return acc_read_reg(0x26);  // Read Filter reference register (resets filter)
}
