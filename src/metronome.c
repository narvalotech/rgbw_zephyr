#include <zephyr.h>
#include "metronome.h"
#include "motor.h"
#include "string.h"

#define TIMER_PERIOD_S 3
#define SAMPLE_NUM 8
#define PULSE_MS 100

uint32_t tempo_ms = 0;
uint32_t samples_ms[SAMPLE_NUM] = {0};

static uint32_t get_average_tempo(void) {
	float average_ms = 0;
	int i = 0;

	for(; i<SAMPLE_NUM; i++) {
		if(samples_ms[i] != 0) {
			average_ms += samples_ms[i];
		}
	}
	average_ms /= (i+1);

	memset(samples_ms, 0, SAMPLE_NUM);

	return (uint32_t)average_ms;
}

struct k_timer metronome_tap_timer;

K_TIMER_DEFINE(metronome_tap_timer, NULL, NULL);

uint32_t metronome_tap_tempo(void)
{
	static uint32_t i = 0;

	uint32_t temp = k_timer_remaining_get(&metronome_tap_timer);

	if(temp != 0) {
		if(i<SAMPLE_NUM)
		{
			samples_ms[i] = (TIMER_PERIOD_S * 1000) - temp;
		} else {
			tempo_ms = get_average_tempo();
			i = 0;
			k_timer_stop(&metronome_tap_timer);
		}
	} else {
		i = 0;
	}

	k_timer_start(&metronome_tap_timer, K_SECONDS(TIMER_PERIOD_S), K_SECONDS(TIMER_PERIOD_S));

	return tempo_ms;
}

void metronome_enable(bool enable)
{
	if (enable) {
		motor_pulse_loop(tempo_ms * 1000, PULSE_MS * 1000);
	} else {
		motor_off();
	}
}
