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

void motor_init(void)
{
	motor = device_get_binding(PWM_LABEL);

	/* Switch off motor */
	pwm_pin_set_usec(motor, PWM_CHANNEL, PWM_PERIOD_US, 0, PWM_FLAGS);
}

void motor_on(uint32_t time_us, uint32_t cycles)
{
	for(; cycles > 0; cycles--)
	{
		/* Switch on a defined intensity */
		pwm_pin_set_usec(motor, PWM_CHANNEL, PWM_PERIOD_US, PWM_PULSE_US, PWM_FLAGS);
		/* Wait a bit */
		k_busy_wait(time_us);
		/* Switch off */
		pwm_pin_set_usec(motor, PWM_CHANNEL, PWM_PERIOD_US, 0, PWM_FLAGS);
	}
}
