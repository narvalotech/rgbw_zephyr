#ifndef __CLOCK_H__
#define __CLOCK_H__

typedef struct
{
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
} time_struct_t;

time_struct_t* clock_get_time_p();
void clock_set_time(time_struct_t time);
void clock_time_init();
void clock_increment_seconds(uint32_t seconds);
void clock_thread_sync();
void clock_thread_unblock();
void clock_set_high_latency(bool latency);

#endif
