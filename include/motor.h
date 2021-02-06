#ifndef __MOTOR_H_
#define __MOTOR_H_

void motor_init(void);
void motor_pulse_single(uint32_t time_us, uint32_t cycles);
void motor_off(void);
void motor_pulse_loop(uint32_t period_us, uint32_t pulse_us);

#endif // __MOTOR_H_
