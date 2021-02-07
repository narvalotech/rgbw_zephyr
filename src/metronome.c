#include <zephyr.h>
#include <string.h>
#include "metronome.h"
#include "motor.h"

#define TIMER_PERIOD_S 3
#define SAMPLE_NUM 8
#define PULSE_MS 500

static uint32_t tempo_ms = 0;
static uint32_t samples_ms[SAMPLE_NUM] = {0};
static uint32_t cur_sample = 0;

static uint32_t get_average_tempo(void) {
	uint64_t average_ms = 0;
	int i = 0;

	for(; (i<SAMPLE_NUM) && samples_ms[i] != 0; i++) {
		average_ms += samples_ms[i];
	}
	average_ms /= (i+1);

	memset(&samples_ms, 0, sizeof(samples_ms));

	return (uint32_t)average_ms;
}

struct k_timer metronome_tap_timer;

static void metronome_tap_timer_callback(struct k_timer *timer_id)
{
	memset(&samples_ms, 0, sizeof(samples_ms));
}

K_TIMER_DEFINE(metronome_tap_timer, metronome_tap_timer_callback, NULL);
uint32_t metronome_tap_tempo(void)
{
	uint32_t temp = k_timer_remaining_get(&metronome_tap_timer);

	if(temp == 0) {
		/* Timer hasn't started yet */
		cur_sample = 0;
		k_timer_start(&metronome_tap_timer,
			      K_SECONDS(TIMER_PERIOD_S),
			      K_SECONDS(TIMER_PERIOD_S));
	} else {
		if(cur_sample < SAMPLE_NUM)
		{
			/* Haven't yet filled all the samples */
			samples_ms[cur_sample] = (TIMER_PERIOD_S * 1000) - temp;
			cur_sample++;
			k_timer_start(&metronome_tap_timer,
				      K_SECONDS(TIMER_PERIOD_S),
				      K_SECONDS(TIMER_PERIOD_S));
		} else {
			tempo_ms = get_average_tempo();
			cur_sample = 0;
			k_timer_stop(&metronome_tap_timer);
		}
	}

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

void metronome_set_tempo(uint32_t bpm) {
	tempo_ms = (bpm * 1000) / 60;
}

uint32_t metronome_get_tempo(void) {
	return (tempo_ms * 60) / 1000;
}
