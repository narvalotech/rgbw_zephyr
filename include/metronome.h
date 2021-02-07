#ifndef __METRONOME_H_
#define __METRONOME_H_

uint32_t metronome_tap_tempo(void);
void metronome_enable(bool enable);
void metronome_set_tempo(uint32_t bpm);
uint32_t metronome_get_tempo(void);

#endif // __METRONOME_H_
