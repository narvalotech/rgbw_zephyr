#ifndef __CLOCK_H__
#define __CLOCK_H__

typedef struct
{
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
} time_struct_t;

time_struct_t* clock_get_time_p();
void clock_set_time(time_struct_t time);
void clock_time_init();
void clock_increment_seconds();
void clock_thread_sync();

#endif
