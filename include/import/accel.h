#ifndef __ACCEL_H__
#define __ACCEL_H__

void accel_test_tilt(void);
int accel_get_mg(int32_t accel[3]);
int accel_init(void);
int accel_high_latency(bool high);

#endif
