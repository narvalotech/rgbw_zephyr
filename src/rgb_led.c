#include <string.h>
#include "rgb_led.h"
#include "nrf_gpio.h"
#include "nrf_spim.h"
#include "nrf_timer.h"
#include "nrf_ppi.h"
#include "board.h"

void s_bus_init(rgb_led_string_config_t* p_led_config)
{
    static uint8_t rx_byte;

    /* Platform-specific code to init comm bus */
    rgb_led_init_gpio(p_led_config);

    /* Start word */
    p_led_config->p_led_data[0].brightness = 0x00;
    p_led_config->p_led_data[0].blue = 0x00;
    p_led_config->p_led_data[0].green = 0x00;
    p_led_config->p_led_data[0].red = 0x00;
    /* End word */
    uint32_t offset = p_led_config->led_num + 1;
    p_led_config->p_led_data[offset].brightness = 0xFF;
    p_led_config->p_led_data[offset].blue = 0xFF;
    p_led_config->p_led_data[offset].green = 0xFF;
    p_led_config->p_led_data[offset].red = 0xFF;

    /* Init SPIM peripheral */
    nrf_spim_pins_set(LED_DRV_SPI,
        p_led_config->pin_clock,
        p_led_config->pin_data,
        0xFFFFFFFF);
    nrf_spim_frequency_set(LED_DRV_SPI, NRF_SPIM_FREQ_1M);
    nrf_spim_configure(LED_DRV_SPI,
                       NRF_SPIM_MODE_0,
                       NRF_SPIM_BIT_ORDER_MSB_FIRST);

    /* Setup TX buffer */
    nrf_spim_tx_buffer_set(LED_DRV_SPI,
                           (uint8_t*)(p_led_config->p_led_data),
                           sizeof(p_led_config->p_led_data));

    /* Setup EasyDMA lists */
    nrf_spim_shorts_enable(LED_DRV_SPI, NRF_SPIM_SHORT_END_START_MASK);
    nrf_spim_tx_list_enable(LED_DRV_SPI);

    /* Track individual led data tx */
    nrf_timer_mode_set(LED_DRV_COUNTER, NRF_TIMER_MODE_COUNTER);
    nrf_timer_bit_width_set(LED_DRV_COUNTER, NRF_TIMER_BIT_WIDTH_16);

    /* Count each led data transfer */
    nrf_ppi_channel_endpoint_setup(LED_DRV_PPI_0,
        nrf_spim_event_address_get(LED_DRV_SPI, NRF_SPIM_EVENT_END),
        (uint32_t)nrf_timer_task_address_get(LED_DRV_COUNTER, NRF_TIMER_TASK_COUNT));
    nrf_ppi_channel_enable(LED_DRV_PPI_0);

    /* Stop SPI auto-transfer when data for all leds has been transferred */
    nrf_ppi_channel_endpoint_setup(LED_DRV_PPI_1,
        (uint32_t)nrf_timer_event_address_get(LED_DRV_COUNTER, NRF_TIMER_EVENT_COMPARE0),
        nrf_spim_task_address_get(LED_DRV_SPI, NRF_SPIM_TASK_STOP));
    nrf_ppi_channel_enable(LED_DRV_PPI_1);

    /* Clear timer when all led data has been transferred */
    nrf_timer_cc_write(LED_DRV_COUNTER, NRF_TIMER_CC_CHANNEL0, p_led_config->led_num + 2);
    nrf_timer_shorts_enable(LED_DRV_COUNTER, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK);

    /* Reset timer & CC0 event */
    nrf_timer_event_clear(LED_DRV_COUNTER, NRF_TIMER_EVENT_COMPARE0);
    nrf_timer_task_trigger(LED_DRV_COUNTER, NRF_TIMER_TASK_START);
    nrf_timer_task_trigger(LED_DRV_COUNTER, NRF_TIMER_TASK_CLEAR);

    /* Don't receive anything */
    nrf_spim_rx_buffer_set(LED_DRV_SPI, &rx_byte, 1);

    /* Enable SPI */
    nrf_spim_enable(LED_DRV_SPI);
}

void rgb_led_init_gpio(rgb_led_string_config_t* p_led_config)
{
    /* Init SCK */
    nrf_gpio_pin_clear(p_led_config->pin_clock);
    nrf_gpio_cfg(p_led_config->pin_clock,
                 NRF_GPIO_PIN_DIR_OUTPUT,
                 NRF_GPIO_PIN_INPUT_CONNECT,
                 NRF_GPIO_PIN_NOPULL,
                 NRF_GPIO_PIN_S0S1,
                 NRF_GPIO_PIN_NOSENSE);
    /* Init SDO */
    nrf_gpio_pin_clear(p_led_config->pin_data);
    nrf_gpio_cfg_output(p_led_config->pin_data);
}

void rgb_led_write(rgb_led_string_config_t* p_led_config)
{
    /* Platform-specific code to transmit led data */
    nrf_timer_event_clear(LED_DRV_COUNTER, NRF_TIMER_EVENT_COMPARE0);
    nrf_spim_event_clear(LED_DRV_SPI, NRF_SPIM_EVENT_ENDTX);
    nrf_spim_event_clear(LED_DRV_SPI, NRF_SPIM_EVENT_END);
    nrf_spim_event_clear(LED_DRV_SPI, NRF_SPIM_EVENT_STARTED);
    nrf_spim_event_clear(LED_DRV_SPI, NRF_SPIM_EVENT_STOPPED);

    /* Reset pointer to start of buffer */
    nrf_spim_tx_buffer_set(LED_DRV_SPI,
                           (uint8_t*)(p_led_config->p_led_data),
                           sizeof(p_led_config->p_led_data));

    nrf_timer_task_trigger(LED_DRV_COUNTER, NRF_TIMER_TASK_CLEAR);
    nrf_spim_shorts_enable(LED_DRV_SPI, NRF_SPIM_SHORT_END_START_MASK);

    /* Start transmitting buffer */
    nrf_spim_task_trigger(LED_DRV_SPI, NRF_SPIM_TASK_START);
    while(!nrf_spim_event_check(LED_DRV_SPI, NRF_SPIM_EVENT_STARTED));
    /* Wait until tx done */
    while(!nrf_spim_event_check(LED_DRV_SPI, NRF_SPIM_EVENT_STOPPED));
    nrf_spim_shorts_disable(LED_DRV_SPI, NRF_SPIM_SHORT_END_START_MASK);
}

void rgb_led_set_global_brightness(rgb_led_string_config_t* p_led_config,
                                   uint8_t global_brightness,
                                   bool write)
{
    global_brightness |= 0xE0;  /* Start of led data MSB is 0b111 */
    p_led_config->brightness = global_brightness;

    /* Set individual brightness levels to global value */
    for(int i=1; i < p_led_config->led_num + 1; i++)
    {
        p_led_config->p_led_data[i].brightness = global_brightness;
    }

    /* Write data to leds */
    if(write)
        rgb_led_write(p_led_config);
}

void rgb_led_set_single_brightness(rgb_led_string_config_t* p_led_config,
                                   uint8_t brightness,
                                   uint32_t led_pos,
                                   bool write)
{
    p_led_config->p_led_data[led_pos + 1].brightness = 0xE0 | brightness;
    if(write)
        rgb_led_write(p_led_config);
}

void rgb_led_set_all(rgb_led_string_config_t* p_led_config,
                     uint8_t red, uint8_t green, uint8_t blue,
                     bool write)
{
    for(int i=1; i < (p_led_config->led_num + 1); i++)
    {
        p_led_config->p_led_data[i].red = red;
        p_led_config->p_led_data[i].green = green;
        p_led_config->p_led_data[i].blue = blue;
    }

    if(write)
        rgb_led_write(p_led_config);
}

void rgb_led_set_led_scale(rgb_led_string_config_t* p_led_config,
                           uint8_t red, uint8_t green, uint8_t blue,
                           uint8_t intensity, uint32_t led_pos,
                           bool write)
{
    red = (uint8_t)((uint32_t)(red * intensity) / 256);
    green = (uint8_t)((uint32_t)(green * intensity) / 256);
    blue = (uint8_t)((uint32_t)(blue * intensity) / 256);

    rgb_led_set_led(p_led_config, red, green, blue, led_pos, write);
}

void rgb_led_set_led(rgb_led_string_config_t* p_led_config,
                     uint8_t red, uint8_t green, uint8_t blue, uint32_t led_pos,
                     bool write)
{
    /* Save data to struct */
    p_led_config->p_led_data[led_pos + 1].red = red;
    p_led_config->p_led_data[led_pos + 1].green = green;
    p_led_config->p_led_data[led_pos + 1].blue = blue;

    /* Write data to bus */
    if(write)
        rgb_led_write(p_led_config);
}

void rgb_led_init(rgb_led_string_config_t* p_led_config)
{
    /* Reset led data */
    memset(p_led_config->p_led_data,
           0,
           sizeof(*p_led_config->p_led_data) * (p_led_config->led_num + 2));

    /* Init physical comm interface */
    s_bus_init(p_led_config);

    /* Set global brightness value */
    rgb_led_set_global_brightness(p_led_config, p_led_config->brightness, false);

    /* Set all leds to black */
    rgb_led_set_all(p_led_config, 0, 0, 0, true);
}
