#include <stdint.h>
#include <stddef.h>
#include "clock.h"

time_struct_t currentTime;

void clock_time_init()
{
	currentTime.hours   = 12;
	currentTime.minutes = 00;
	currentTime.seconds = 00;
}

time_struct_t* clock_get_time_p()
{
	return (time_struct_t*)(&currentTime);
}

void clock_set_time(time_struct_t newTime)
{
	currentTime.hours   = newTime.hours;
	currentTime.minutes = newTime.minutes;
	currentTime.seconds = newTime.seconds;
}

void clock_increment_seconds()
{
	currentTime.seconds++;
	if(currentTime.seconds == 60) 
	{
		currentTime.minutes++;
		currentTime.seconds = 0;
	}
	if(currentTime.minutes == 60) 
	{
		currentTime.hours++;
		currentTime.minutes = 0;
	}
	if(currentTime.hours == 24)
		currentTime.hours = 0;
}
