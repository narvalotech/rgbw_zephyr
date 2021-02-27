#include <zephyr.h>
#include <string.h>
#include "metronome.h"
#include "motor.h"

#define TIMER_PERIOD_S 3

volatile uint32_t tempo_ms = 0;

struct k_timer metronome_tap_timer;
K_TIMER_DEFINE(metronome_tap_timer, NULL, NULL);

uint32_t metronome_tap_tempo(void)
{
	tempo_ms = (TIMER_PERIOD_S * 1000)
		- k_timer_remaining_get(&metronome_tap_timer);
	motor_loop(tempo_ms, 0);

	k_timer_start(&metronome_tap_timer,
			K_SECONDS(TIMER_PERIOD_S),
			K_MSEC(0));

	return tempo_ms;
}

void metronome_enable(bool enable)
{
	if (enable) {
		motor_loop(tempo_ms, 1);
	} else {
		motor_off();
	}
}

void metronome_set_tempo(uint32_t bpm) {
	tempo_ms = 1000000 / ((bpm * 1000) / 60);
	motor_loop(tempo_ms, 0);
}

uint32_t metronome_get_tempo(void) {
	return (1000000 / (tempo_ms * 100)) * 60;
}
