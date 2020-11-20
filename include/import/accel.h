#ifndef __ACCEL_H__
#define __ACCEL_H__

#include <stdbool.h>
#include <stdint.h>

//----Accel sample rate: ctrl_1 ODR bits----
#define ACC_RATE_1       1
#define ACC_RATE_10      2
#define ACC_RATE_25      3
#define ACC_RATE_50      4
#define ACC_RATE_100     5
#define ACC_RATE_200     6
#define ACC_RATE_400     7
#define ACC_RATE_1250    9
#define ACC_RATE_1600_LP 8
#define ACC_RATE_5000_LP 9

//----Accel Full-scale value: ctrl_4 FS bits----
#define ACC_FS_2G  0
#define ACC_FS_4G  1
#define ACC_FS_8G  2
#define ACC_FS_16G 3

//----Interrupt Generator 1 modes----
#define IG_OR     0
#define IG_6D_MOV 0x40
#define IG_AND    0x80
#define IG_6D_POS 0xC0

//----Intgen axes----
#define I1_ZH 0x20
#define I1_ZL 0x10
#define I1_YH 0x08
#define I1_YL 0x04
#define I1_XH 0x02
#define I1_XL 0x01
#define I1_INT 0x40

//----Tap Interrupt Axes / Modes
#define TAP_ZD 0x20
#define TAP_ZS 0x10
#define TAP_YD 0x08
#define TAP_YS 0x04
#define TAP_XD 0x02
#define TAP_XS 0x01

//----Tap Interrupt Sources
#define TAP_INT    0x40
#define TAP_D      0x20
#define TAP_S      0x10
#define TAP_SIGN   0x08
#define TAP_Z      0x04
#define TAP_Y      0x02
#define TAP_X      0x01

//----I1 Interrupt Sources----
#define I1_CLICK     0x80
#define I1_IG1       0x40
#define I1_IG2       0x20
#define I1_DRDY1     0x10
#define I1_DRDY2     0x08
#define I1_WTM       0x04
#define I1_OVERRUN   0x02

//----I2 Interrupt Sources----
#define I2_CLICK     0x80
#define I2_IG1       0x40
#define I2_BOOT      0x10
#define I2_HLACTIVE  0x02

//----High-pass Filter Configuration----
#define HPF_0   0x10
#define HPF_1   0x20
#define HPM_0   0x40
#define HPM_1   0x80
#define HPDATA  0x08
#define HPCLICK 0x04
#define HPINT2  0x02
#define HPINT1  0x01

#define HP_MODE_NORMAL_RESET 0x00
#define HP_MODE_REF          0x40
#define HP_MODE_NORMAL       0x80
#define HP_MODE_AUTORESET    0xC0

//----Navigation defines----
#define LEFT   1
#define RIGHT  2
#define UP     3
#define DOWN   4
#define CLICK  5
#define DCLICK 6

typedef struct 
{
	int16_t x;
	int16_t y;
	int16_t z;
} mvm_struct_t;

uint8_t acc_read_reg(uint8_t reg);
void acc_write_reg(uint8_t reg, uint8_t data);

// Measurement fetch
void acc_read3(int16_t* x, int16_t* y, int16_t* z);
int acc_read_x();
int acc_read_y();
int acc_read_z();

// Init and configuration
void acc_init(bool sa0);
void acc_lowpower(uint8_t mode);
void acc_rate(uint8_t rate);
void acc_scale(uint8_t scale);
void acc_highres(uint8_t highres);

// INT1 Interrupt configuration
void acc_intgen_config(uint8_t config);
void acc_intgen_duration(uint16_t ms);
void acc_intgen_threshold(uint16_t th_mg);
uint8_t acc_intgen_event();

// Click/Tap Interrupt configuration
void acc_click_set(uint8_t sources, 
					uint16_t threshold, 
					uint16_t limit_ms, 
					uint16_t latency_ms, 
					uint16_t window_ms);
uint8_t acc_click_event();

// Interrupt pins 1 and 2 config
void acc_int1_sources(uint8_t sources);
void acc_int2_sources(uint8_t sources);

// High-Pass Filter
void acc_hpf_config(uint8_t config);
void acc_hpf_set_ref(uint16_t ref);
uint8_t acc_hpf_reset();


#endif
