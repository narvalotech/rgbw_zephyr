#include <stdint.h>
#include <stddef.h>
#include "nrf_rtc.h"
#include "nrf_ppi.h"
#include "clock.h"
#include "board.h"
#include "disp.h"

time_struct_t currentTime;

void RTC2_IRQHandler(void)
{
	if(CLOCK_RTC->EVENTS_COMPARE[0])
	{
		nrf_rtc_event_clear(CLOCK_RTC, NRF_RTC_EVENT_COMPARE_0);
		clock_increment_seconds();
	}
}

void s_rtc_init()
{
	/* Auto-restart on CC[0] */
	nrf_ppi_channel_endpoint_setup(RTC_DRV_PPI,
								   nrf_rtc_event_address_get(CLOCK_RTC, NRF_RTC_EVENT_COMPARE_0),
								   nrf_rtc_task_address_get(CLOCK_RTC, NRF_RTC_TASK_CLEAR));
	nrf_ppi_channel_enable(RTC_DRV_PPI);

	/* ISR every second */
	nrf_rtc_cc_set(CLOCK_RTC, 0, 32767);

	nrf_rtc_event_clear(CLOCK_RTC, NRF_RTC_EVENT_COMPARE_0);
	nrf_rtc_event_enable(CLOCK_RTC, RTC_CHANNEL_INT_MASK(0));
	nrf_rtc_int_enable(CLOCK_RTC, RTC_CHANNEL_INT_MASK(0));
	sd_nvic_SetPriority(CLOCK_IRQ_NUM, CLOCK_IRQ_PRIO);
	sd_nvic_EnableIRQ(CLOCK_IRQ_NUM);

	nrf_rtc_task_trigger(CLOCK_RTC, NRF_RTC_TASK_CLEAR);
	nrf_rtc_task_trigger(CLOCK_RTC, NRF_RTC_TASK_START);
}

void clock_rtc_poweroff()
{
	nrf_ppi_channel_disable(RTC_DRV_PPI);
	nrf_rtc_task_trigger(CLOCK_RTC, NRF_RTC_TASK_STOP);
	nrf_rtc_event_clear(CLOCK_RTC, NRF_RTC_EVENT_COMPARE_0);
	nrf_rtc_event_disable(CLOCK_RTC, RTC_CHANNEL_INT_MASK(0));
	sd_nvic_DisableIRQ(CLOCK_IRQ_NUM);
}

void clock_time_init()
{
	s_rtc_init();
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
