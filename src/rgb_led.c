#include <string.h>
#include <drivers/spi.h>
#include "rgb_led.h"

static const struct device *spi;
static struct spi_config spi_cfg;
static struct spi_buf spi_tx_buf;
const static struct spi_buf_set spi_tx_buf_set = {
    .buffers = &spi_tx_buf,
    .count = 1,
};

void s_bus_init(rgb_led_string_config_t* p_led_config)
{
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

    /* Store pointer to main buffer */
    spi_tx_buf.buf = p_led_config->p_led_data;
    spi_tx_buf.len = (p_led_config->led_num + 2) * 4;

    /* Init zephyr spi peripheral */
    spi = device_get_binding(DT_LABEL(DT_ALIAS(ledspi)));
    /* if(!spi) { */
    /*     return -ENODEV; */
    /* } */
    spi_cfg.slave = 0;
    spi_cfg.frequency = 1000000;
    spi_cfg.operation =
        SPI_OP_MODE_MASTER
        | SPI_TRANSFER_MSB
        | SPI_WORD_SET(8);
}

void rgb_led_init_gpio(rgb_led_string_config_t* p_led_config)
{
    /* Not implemented for now */
    (void)p_led_config;
    return;
}

void rgb_led_write(rgb_led_string_config_t* p_led_config)
{
    /* Blocking write */
    spi_write(spi, &spi_cfg, &spi_tx_buf_set);
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
