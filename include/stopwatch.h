#ifndef __STOPWATCH_H_
#define __STOPWATCH_H_

#include <stdint.h>

typedef enum {
    STW_STOPPED = 0,
    STW_STARTED,
} stopwatch_state_t;

typedef struct {
	uint64_t ms;		/* Total time in ms */
	uint8_t cents;
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;		/* 255 hours ought to be enough */
} stopwatch_time_t;

void stopwatch_init(uint8_t channel);
void stopwatch_thread_sync(uint8_t channel);

void stopwatch_start(uint8_t channel);
void stopwatch_stop(uint8_t channel);
void stopwatch_reset(uint8_t channel);
/* Toggle between start/stop */
stopwatch_state_t stopwatch_toggle(uint8_t channel);
stopwatch_state_t stopwatch_state_get(uint8_t channel);

stopwatch_time_t* stopwatch_time_get(uint8_t channel);
uint32_t stopwatch_ms_get(uint8_t channel);
uint8_t stopwatch_hours_get(uint8_t channel);
uint8_t stopwatch_minutes_get(uint8_t channel);
uint8_t stopwatch_seconds_get(uint8_t channel);
uint8_t stopwatch_cents_get(uint8_t channel);

#endif // __STOPWATCH_H_
