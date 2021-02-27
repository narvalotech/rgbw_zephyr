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

typedef enum {
	STOP = 0,
	OVERDRIVE,
	RUN,
} motor_state_t;

const uint32_t m_duty[3] =    { 0, 100, 60};
const uint32_t m_time_ms[3] = { 0, 25, 150};

volatile uint32_t period_ms = 1000;
volatile motor_state_t motor_state = 0;

const struct device *motor;

struct k_timer motor_timer;

static void motor_set_duty(uint32_t duty)
{
	pwm_pin_set_usec(motor, PWM_CHANNEL,
			 PWM_PERIOD_US, (duty * PWM_PERIOD_US) / 100,
			 PWM_FLAGS);
}

static void motor_timer_callback(struct k_timer *timer_id)
{
	motor_set_duty(m_duty[motor_state]);

	switch (motor_state) {
		case OVERDRIVE:
			k_timer_start(&motor_timer,
				      K_MSEC(m_time_ms[motor_state]),
				      K_MSEC(0));
			motor_state = RUN;
			break;
		case RUN:
			k_timer_start(&motor_timer,
				      K_MSEC(m_time_ms[motor_state]),
				      K_MSEC(0));
			motor_state = STOP;
			break;
		case STOP:
		{
			uint32_t rem_time_ms = 0;
			if (period_ms > (m_time_ms[1] + m_time_ms[2])) {
				rem_time_ms =
					period_ms - m_time_ms[1] - m_time_ms[2];
			}
			k_timer_start(&motor_timer,
				      K_MSEC(rem_time_ms),
				      K_MSEC(0));
			motor_state = OVERDRIVE;
			break;
		}
		default:
			return;
	}
}
K_TIMER_DEFINE(motor_timer, motor_timer_callback, NULL);

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
	k_timer_stop(&motor_timer);
	motor_state = STOP;
	pwm_pin_set_usec(motor, PWM_CHANNEL, PWM_PERIOD_US, 0, PWM_FLAGS);
}

void motor_loop(uint32_t loop_time_ms, bool start_timer)
{
	period_ms = loop_time_ms;
	if(start_timer)
	{
		motor_state = OVERDRIVE;
		k_timer_start(&motor_timer, K_MSEC(1), K_MSEC(0));
	}
}
