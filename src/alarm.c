#include <zephyr.h>
#include <string.h>
#include <init.h>
#include "calendar.h"
#include "clock.h"
#include "motor.h"
#include "state.h"
#include "alarm.h"

#define PULSE_TIME_US (250*1000)
#define PULSE_TIMES 1
#define PULSE_INTERVAL K_SECONDS(2)

struct k_work alarm_work;
static alarm_struct_t alarm_time;
static bool alarm_enabled = 0;

extern struct g_state state;

struct k_timer alarm_timer;
static void alarm_timer_callback(struct k_timer *timer_id)
{
	k_work_submit(&alarm_work);
}
K_TIMER_DEFINE(alarm_timer, alarm_timer_callback, NULL);

static void ring(struct k_work *item)
{
	ARG_UNUSED(item);
	motor_pulse_single(PULSE_TIME_US, PULSE_TIMES);
}

void alarm_set(alarm_struct_t * p_alarm_time)
{
	memcpy(&alarm_time, p_alarm_time, sizeof(alarm_time));
	memcpy(&alarm_time.time_sn,
	       &p_alarm_time->time,
	       sizeof(time_struct_t));
}

void alarm_get(alarm_struct_t * p_alarm_time)
{
	memcpy(p_alarm_time, &alarm_time, sizeof(alarm_time));
}

bool alarm_check(void)
{
	if (!alarm_enabled)
		return false;

	if (!((1 << cal_get_weekday(NULL)) & alarm_time.days))
		return false;

	time_struct_t *p_time = clock_get_time_p();
	if ((p_time->hours == alarm_time.time_sn.hours &&
	     (p_time->minutes == alarm_time.time_sn.minutes))) {
		/* Show alarm screen */
		state.next = 1;
		state.pgm_state = PGM_STATE_ALARM_RING;
		/* Wake from sleep to show alarm screen and to begin
		 * ringing. */
		k_wakeup(state.main_tid);
		return true;
	}

	return false;
}

void alarm_enable(bool enable)
{
	alarm_enabled = enable;
}

bool alarm_is_enabled(void)
{
	return alarm_enabled;
}

void alarm_snooze(uint8_t minutes) {
	/* Still doesn't handle day-rollover tho */
	alarm_time.time_sn.minutes += minutes;
	if(alarm_time.time_sn.minutes >= 60) {
		alarm_time.time_sn.hours += 1;
		alarm_time.time_sn.minutes -= 60;
	}

	/* Cut the power to the motor */
	k_timer_stop(&alarm_timer);
}

void alarm_stop() {
	/* Reset snooze-adjusted alarm time */
	memcpy(&alarm_time.time_sn,
	       &alarm_time.time,
	       sizeof(time_struct_t));

	/* Cut the power to the motor */
	k_timer_stop(&alarm_timer);
}

void alarm_start(void) {
	/* Ring alarm (motor) */
	k_work_submit(&alarm_work);
	k_timer_start(&alarm_timer, PULSE_INTERVAL, PULSE_INTERVAL);
}

static int alarm_init(const struct device *dev) {
	k_work_init(&alarm_work, ring);

	return 0;
}
SYS_INIT(alarm_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
