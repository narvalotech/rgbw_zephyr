#ifndef __STATE_H_
#define __STATE_H_

#include <stdbool.h>
#include <stdint.h>
#include <kernel.h>

typedef enum {
	PGM_STATE_TEST_TILT,
	PGM_STATE_CLOCK,
	PGM_STATE_CLOCK_SET,
	PGM_STATE_STOPWATCH,
	PGM_STATE_COUNTDOWN,
	PGM_STATE_ALARM,
	PGM_STATE_ALARM_RING,
	PGM_STATE_CHARGE,
	PGM_STATE_BATT,
	PGM_STATE_BLE,
	PGM_STATE_DFU_END
} pgm_state_t;

struct g_state {
	k_tid_t main_tid;	/* Main thread ID */
	pgm_state_t pgm_state;	/* Current program state */
	bool main;		/* Go back to main screen */
	bool next;		/* Go to next screen/activity */
	bool abort;	        /* Abort current procedure */
	bool motion_wake;	/* Enable/disable motion wakeup */
	uint8_t but_ll;		/* Lower left button pressed */
	uint8_t but_lr;		/* Lower right button pressed */
	uint8_t but_ur;		/* Upper right button pressed */
	/* TODO: clean this up */
};

void state_clear(void);
void main_state_set(pgm_state_t curr_state, pgm_state_t new_state);
void main_state_loop(void);
bool state_is_button_pressed(void);

#endif // __STATE_H_
