#include <stdint.h>
#include <stdbool.h>
#include "screen.h"
#include "state.h"

extern struct g_state state;

void state_clear(void)
{
	/* Clear all state except main/hibernate/pgm */
	state.abort_disp = 0;
	state.exit_signal = 0;
	state.select = 0;
	state.but_ll = 0;
	state.but_lr = 0;
	state.but_ur = 0;
	state.but_long_press = 0;
}

void main_state_set(pgm_state_t curr_state, pgm_state_t new_state)
{
	if(state.main) {
		state.pgm_state = PGM_STATE_CLOCK_BCD;
		state.main = 0;
	}
	else if(state.hibernate) {
		state.pgm_state = PGM_STATE_HIBERNATE;
		state.hibernate = 0;
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
			main_state_set(PGM_STATE_TEST_TILT, PGM_STATE_CLOCK_BCD);
			break;
		case PGM_STATE_CLOCK_BCD:
			screen_clock_bcd();
			main_state_set(PGM_STATE_CLOCK_BCD, PGM_STATE_STOPWATCH);
			break;
		case PGM_STATE_CLOCK_DIGITAL:
			/* TODO: add digital screen */
			break;
		case PGM_STATE_STOPWATCH:
			screen_stopwatch();
			main_state_set(PGM_STATE_STOPWATCH, PGM_STATE_METRONOME);
			break;
		case PGM_STATE_METRONOME:
			screen_metronome();
			/* main_state_set(PGM_STATE_METRONOME, PGM_STATE_TEST); */
			main_state_set(PGM_STATE_METRONOME, PGM_STATE_BLE);
			break;
		case PGM_STATE_CLOCK_SET:
			screen_time_set();
			/* main_state_set(PGM_STATE_TEST); */
			main_state_set(PGM_STATE_CLOCK_SET, PGM_STATE_CLOCK_BCD);
			break;
		/* case PGM_STATE_TEST: */
		/* 	screen_test(); */
		/* 	main_state_set(PGM_STATE_TEST, PGM_STATE_BATT); */
		/* 	break; */
		/* case PGM_STATE_BATT: */
		/* 	screen_battery(); */
		/* 	main_state_set(PGM_STATE_BATT, PGM_STATE_BLE); */
		/* 	break; */
		case PGM_STATE_BLE:
			screen_ble();
			main_state_set(PGM_STATE_BLE, PGM_STATE_TEST_TILT);
			break;
		case PGM_STATE_CHARGE:
			/* TODO: add charging screen */
			break;
		case PGM_STATE_HIBERNATE:
			/* TODO: add hibernating routine */
			break;
		default:
			break;
	}

	return;
}
