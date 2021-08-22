#include <stdint.h>
#include <stdbool.h>
#include "screen.h"
#include "state.h"

extern struct g_state state;

bool state_is_button_pressed(void)
{
	return (state.but_ll || state.but_lr || state.but_ur);
}

void state_clear(void)
{
	/* Clear all state except main/pgm */
	state.abort = 0;
	state.next = 0;
	state.but_ll = 0;
	state.but_lr = 0;
	state.but_ur = 0;
}

void main_state_set(pgm_state_t curr_state, pgm_state_t new_state)
{
	if(state.main) {
		state.pgm_state = PGM_STATE_CLOCK;
		state.main = 0;
	}
	/* If next state has been set from somewhere else,
	 * don't mess with it. */
	else if(state.pgm_state == curr_state) {
		state.pgm_state = new_state;
	}
}

void main_state_loop(void)
{
	switch (state.pgm_state)
	{
		case PGM_STATE_TEST_TILT:
			screen_test_tilt();
			main_state_set(PGM_STATE_TEST_TILT, PGM_STATE_CLOCK);
			break;
		case PGM_STATE_CLOCK:
			screen_clock();
			main_state_set(PGM_STATE_CLOCK, PGM_STATE_STOPWATCH);
			break;
		case PGM_STATE_STOPWATCH:
			screen_stopwatch();
			main_state_set(PGM_STATE_STOPWATCH, PGM_STATE_COUNTDOWN);
			break;
		case PGM_STATE_COUNTDOWN:
			screen_countdown();
			main_state_set(PGM_STATE_COUNTDOWN, PGM_STATE_ALARM);
			break;
		case PGM_STATE_ALARM:
			screen_alarm_view();
			main_state_set(PGM_STATE_ALARM, PGM_STATE_BLE);
			break;
		case PGM_STATE_ALARM_RING:
			screen_alarm_ring();
			main_state_set(PGM_STATE_ALARM_RING, PGM_STATE_CLOCK);
			break;
		case PGM_STATE_CLOCK_SET:
			screen_time_set();
			/* main_state_set(PGM_STATE_TEST); */
			main_state_set(PGM_STATE_CLOCK_SET, PGM_STATE_CLOCK);
			break;
		/* case PGM_STATE_BATT: */
		/* 	screen_battery(); */
		/* 	main_state_set(PGM_STATE_BATT, PGM_STATE_BLE); */
		/* 	break; */
		case PGM_STATE_BLE:
			screen_ble();
			main_state_set(PGM_STATE_BLE, PGM_STATE_CLOCK);
			break;
		case PGM_STATE_DFU_END:
			screen_dfu_end();
			/* No exiting from this state */
			break;
		case PGM_STATE_CHARGE:
			/* TODO: add charging screen */
			break;
		default:
			break;
	}

	return;
}
