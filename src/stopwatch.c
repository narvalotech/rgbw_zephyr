#include <stdint.h>
#include <stddef.h>
#include <zephyr.h>
#include "clock.h"
#include "stopwatch.h"

void stopwatch_init(uint8_t channel)
{
	ARG_UNUSED(channel);
	return;
}

void stopwatch_thread_sync(uint8_t channel)
{
	ARG_UNUSED(channel);
	return;
}

void stopwatch_start(uint8_t channel)
{
	ARG_UNUSED(channel);
	return;
}

void stopwatch_stop(uint8_t channel)
{
	ARG_UNUSED(channel);
	return;
}

stopwatch_state_t stopwatch_toggle(uint8_t channel)
{
	ARG_UNUSED(channel);
	return 0;
}

void stopwatch_reset(uint8_t channel)
{
	ARG_UNUSED(channel);
	return;
}

stopwatch_state_t stopwatch_state_get(uint8_t channel)
{
	ARG_UNUSED(channel);
	return 0;
}

uint32_t stopwatch_ms_get(uint8_t channel)
{
	ARG_UNUSED(channel);
	return 0;
}
