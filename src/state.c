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
}

void main_state_set(pgm_state_t new_state)
{
	if(state.main) {
		state.pgm_state = PGM_STATE_CLOCK_BCD;
	}
	else if(state.hibernate) {
		state.pgm_state = PGM_STATE_HIBERNATE;
	}
	else {
		state.pgm_state = new_state;
	}
}

void main_state_loop(void)
{
	switch (state.pgm_state)
	{
		case PGM_STATE_TEST_TILT:
			screen_test_tilt();
			main_state_set(PGM_STATE_CLOCK_BCD);
			break;
		case PGM_STATE_CLOCK_BCD:
			screen_clock_bcd();
			main_state_set(PGM_STATE_STOPWATCH);
			break;
		case PGM_STATE_CLOCK_DIGITAL:
			/* TODO: add digital screen */
			break;
		case PGM_STATE_STOPWATCH:
			screen_stopwatch();
			main_state_set(PGM_STATE_CLOCK_SET);
			break;
		case PGM_STATE_CLOCK_SET:
			screen_time_set();
			main_state_set(PGM_STATE_CLOCK_BCD);
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
