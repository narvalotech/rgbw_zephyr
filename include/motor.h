#ifndef __MOTOR_H_
#define __MOTOR_H_

void motor_init(void);
void motor_pulse_single(uint32_t time_us, uint32_t cycles);
void motor_pulse_async(uint32_t time_us, uint32_t cycles);
void motor_off(void);
void motor_loop(uint32_t loop_time_ms, bool start_timer);

#endif // __MOTOR_H_
