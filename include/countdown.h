#ifndef __COUNTDOWN_H_
#define __COUNTDOWN_H_

#include "clock.h"		/* For time_struct_t */

/* Countdown timer module */

typedef enum {
    CD_TIMER_STOPPED = 0,
    CD_TIMER_STARTED,
} cd_timer_state_t;

/* pass in initial timer value */
void cd_timer_start(time_struct_t* p_time);

void cd_timer_stop(void);
void cd_timer_remaining_get(time_struct_t* time);

/* register a callback to be called on timer expiry */
void cd_timer_expiry_register_fn(void (*callback_fn)(void));

cd_timer_state_t cd_timer_state_get(void);

#endif // __COUNTDOWN_H_
