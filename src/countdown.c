#include <zephyr.h>
#include <init.h>
#include <string.h>
#include "clock.h"		/* For time_struct_t */
#include "motor.h"
#include "countdown.h"

#define PULSE_TIME_MS 500
static cd_timer_state_t state = CD_TIMER_STOPPED;
struct k_timer cd_timer;
static void (*user_callback)(void);
struct k_work cd_timer_work;

static uint32_t time_to_s(time_struct_t* p_time)
{
	uint32_t time_s =
		(p_time->seconds) +
		(p_time->minutes * 60) +
		(p_time->hours * 60 * 60);

	return time_s;
}

/* Order of expressions matters */
#pragma GCC optimize("O0")
static void s_to_time(uint32_t seconds, time_struct_t* p_time)
{
	p_time->hours = seconds / (60 * 60);
	seconds -= p_time->hours * 60 * 60;

	p_time->minutes = seconds / 60;
	seconds -= p_time->minutes * 60;

	p_time->seconds = seconds;
}

static void cd_timer_callback(struct k_timer *timer_id)
{
	/* Call user-supplied callback */
	if(user_callback) {
		user_callback();
	}
	k_work_submit(&cd_timer_work);
	cd_timer_stop();
}

K_TIMER_DEFINE(cd_timer, cd_timer_callback, NULL);

static void alert_user(struct k_work *item)
{
	ARG_UNUSED(item);
	motor_pulse_single(PULSE_TIME_MS * 1000, 2);
}

void cd_timer_thread_sync(void)
{
	k_timer_status_sync(&cd_timer);
}

void cd_timer_start(time_struct_t* p_time)
{
	/* Start timer */
	k_timer_start(&cd_timer, K_SECONDS(time_to_s(p_time)), K_NO_WAIT);
	state = CD_TIMER_STARTED;
}

void cd_timer_stop(void)
{
	k_timer_stop(&cd_timer);
	state = CD_TIMER_STOPPED;
}

void cd_timer_remaining_get(time_struct_t* p_time)
{
	uint32_t rem_sec = k_timer_remaining_get(&cd_timer) / 1000;
	s_to_time(rem_sec, p_time);
}

void cd_timer_expiry_register_fn(void (*callback_fn)(void))
{
	if(callback_fn) {
		user_callback = callback_fn;
	}
}

cd_timer_state_t cd_timer_state_get(void)
{
	return state;
}

static int cd_timer_init(const struct device *dev)
{
	ARG_UNUSED(dev);
	k_work_init(&cd_timer_work, alert_user);

	return 0;
}

SYS_INIT(cd_timer_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
