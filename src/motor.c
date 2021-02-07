#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/pwm.h>
#include "motor.h"

#define PWM_MOTOR_NODE DT_NODELABEL(hapt_pwm)
#define PWM_LABEL DT_PWMS_LABEL(PWM_MOTOR_NODE)
#define PWM_CHANNEL DT_PWMS_CHANNEL(PWM_MOTOR_NODE)
#define PWM_FLAGS DT_PWMS_FLAGS(PWM_MOTOR_NODE)
#define PWM_PERIOD_US (16 * 10)
#define PWM_PULSE_US (PWM_PERIOD_US / 2)

const struct device *motor;
static uint32_t s_period_us = 0;
static uint32_t s_pulse_us = 0;

#define MOTOR_STACK_SIZE 500
#define MOTOR_PRIORITY 5

static void motor_entry_point(void *p1, void *p2, void *p3);
K_THREAD_DEFINE(motor_tid, MOTOR_STACK_SIZE,
                motor_entry_point, NULL, NULL, NULL,
                MOTOR_PRIORITY, 0, 0);

void motor_init(void)
{
	motor = device_get_binding(PWM_LABEL);

	/* Switch off motor */
	pwm_pin_set_usec(motor, PWM_CHANNEL, PWM_PERIOD_US, 0, PWM_FLAGS);
}

void motor_pulse_single(uint32_t time_us, uint32_t cycles)
{
	for(; cycles > 0; cycles--)
	{
		/* Switch on a defined intensity */
		pwm_pin_set_usec(motor, PWM_CHANNEL, PWM_PERIOD_US, PWM_PULSE_US, PWM_FLAGS);
		/* Wait a bit */
		k_busy_wait(time_us / 2);
		/* Switch off */
		pwm_pin_set_usec(motor, PWM_CHANNEL, PWM_PERIOD_US, 0, PWM_FLAGS);
		/* Wait a bit */
		if(cycles>1)
			k_busy_wait(time_us / 2);
	}
}

void motor_off(void)
{
	k_thread_suspend(k_current_get());
	pwm_pin_set_usec(motor, PWM_CHANNEL, PWM_PERIOD_US, 0, PWM_FLAGS);
}

void motor_pulse_loop(uint32_t period_us, uint32_t pulse_us)
{
	s_period_us = period_us;
	s_pulse_us = pulse_us;

	k_thread_resume(motor_tid);
}

static void motor_entry_point(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	/* Disable on startup */
	k_thread_suspend(k_current_get());

	while (1) {
		pwm_pin_set_usec(motor, PWM_CHANNEL, s_period_us, s_pulse_us,
				 PWM_FLAGS);
		k_msleep(100);
	}
}
