#ifndef __STOPWATCH_H_
#define __STOPWATCH_H_

#include <stdint.h>

typedef enum {
    STW_STOPPED = 0,
    STW_STARTED,
} stopwatch_state_t;

void stopwatch_init(uint8_t channel);
void stopwatch_thread_sync(uint8_t channel);

void stopwatch_start(uint8_t channel);
void stopwatch_stop(uint8_t channel);
void stopwatch_reset(uint8_t channel);
/* Toggle between start/stop */
stopwatch_state_t stopwatch_toggle(uint8_t channel);
stopwatch_state_t stopwatch_state_get(uint8_t channel);

uint32_t stopwatch_ms_get(uint8_t channel);

#endif // __STOPWATCH_H_
