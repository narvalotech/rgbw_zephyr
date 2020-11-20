#include "power.h"
#include "nrf.h"
#include "board.h"
#include "disp.h"
#include "clock.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
/* Softdevice */
#include "nrf_soc.h"
#include "nrf_sdm.h"
#include "nrf_sdh.h"

void power_system_sleep()
{
    display_clear();
    led_vdd_enable(0);
    power_periph_shutdown();
    /* TODO: put accel to sleep */
    sd_power_mode_set(NRF_POWER_MODE_LOWPWR);
}

void power_system_wakeup()
{
    power_periph_wakeup();
    led_vdd_enable(1);
    /* TODO: add DCDC startup delay */
    display_clear();
}

void power_periph_shutdown()
{
    /* Shut down communication interfaces */
    LED_DRV_SPI->ENABLE = 0;
    I2C_DRV_TWI->ENABLE = 0;

    /* Shutdown timer (16M source) */
    LED_DRV_COUNTER->TASKS_STOP = 1;
}

void power_periph_wakeup()
{
    /* Wakeup communication interfaces */
    LED_DRV_SPI->ENABLE = 1;
    I2C_DRV_TWI->ENABLE = 1;

    /* Wakeup timer (16M source) */
    LED_DRV_COUNTER->TASKS_START = 1;
}

void power_hibernate(bool initialized)
{
    if(!initialized)
    {
        led_vdd_enable(0);
        sensor_vdd_enable(0);
        NRF_POWER->GPREGRET = 0;
        /* Enter systemoff */
        NRF_POWER->SYSTEMOFF = 1;
    }
    else
    {
        /* Power off display & 5V rail */
        display_clear();
        led_vdd_enable(0);

        /* Power off external/sensor VDD rail */
        sensor_vdd_enable(0);

        /* Shut down communication interfaces */
        LED_DRV_SPI->ENABLE = 0;
        I2C_DRV_TWI->ENABLE = 0;

        /* Shutdown timers */
        LED_DRV_COUNTER->TASKS_STOP = 1;
        LED_DRV_COUNTER->TASKS_SHUTDOWN = 1;
        clock_rtc_poweroff();

        /* Soft-reset will trigger entering the !initialized state
           and resets all peripherals / advertising for us */
        NRF_POWER->GPREGRET = 0xfc;
        NVIC_SystemReset();
    }
}

void led_vdd_enable(bool enable)
{
    nrf_gpio_cfg_output(PIN_FP_5V_EN);
    if(enable)
        nrf_gpio_pin_set(PIN_FP_5V_EN);
    else
        nrf_gpio_pin_clear(PIN_FP_5V_EN);
}

void sensor_vdd_enable(bool enable)
{
    nrf_gpio_cfg_output(PIN_SENS_EN);
    if(enable)
        nrf_gpio_pin_set(PIN_SENS_EN);
    else
        nrf_gpio_pin_clear(PIN_SENS_EN);
}
