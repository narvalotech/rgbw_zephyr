#ifndef __RGB_LED_H_
#define __RGB_LED_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint8_t brightness;
    uint8_t blue;
    uint8_t green;
    uint8_t red;
} rgb_led_value_t;

typedef struct
{
    rgb_led_value_t* p_led_data; /* Pointer to led data array */
    uint32_t led_num;            /* Number of leds on bus */
    uint8_t brightness;          /* Brightness value applied to whole string */
    uint32_t pin_data;           /* Also called SDI */
    uint32_t pin_clock;          /* Also called SCI */
} rgb_led_string_config_t;


void s_bus_init(rgb_led_string_config_t* p_led_config);

void rgb_led_init_gpio(rgb_led_string_config_t* p_led_config);

void rgb_led_write(rgb_led_string_config_t* p_led_config);

void rgb_led_set_global_brightness(rgb_led_string_config_t* p_led_config,
                                   uint8_t global_brightness,
                                   bool write);

void rgb_led_set_single_brightness(rgb_led_string_config_t* p_led_config,
                                   uint8_t brightness,
                                   uint32_t led_pos,
                                   bool write);

void rgb_led_set_all(rgb_led_string_config_t* p_led_config,
                     uint8_t red, uint8_t green, uint8_t blue,
                     bool write);

void rgb_led_set_led(rgb_led_string_config_t* p_led_config,
                     uint8_t red, uint8_t green, uint8_t blue, uint32_t led_pos,
                     bool write);

void rgb_led_set_led_scale(rgb_led_string_config_t* p_led_config,
                           uint8_t red, uint8_t green, uint8_t blue,
                           uint8_t intensity, uint32_t led_pos,
                           bool write);

void rgb_led_init(rgb_led_string_config_t* p_led_config);

#endif // __RGB_LED_H_
