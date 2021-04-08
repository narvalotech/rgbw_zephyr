#ifndef __DISP_H__
#define __DISP_H__

#define DISPLAY_BCD 1
#define DISPLAY_DIGITAL 2

#define DISP_FX_FADE 0
#define DISP_FX_SLIDE 1
#define DISP_FX_DIR_LEFT 0
#define DISP_FX_DIR_RIGHT 1
#define DISP_FX_DIR_IN 1
#define DISP_FX_DIR_OUT 0

void display_init();
void display_refresh();
void display_clear();
void display_fade(bool dir, uint32_t fade_time);
void display_fade_next(bool dir, uint32_t fade_time, uint8_t fx_type);
void display_bytes(int top, int mid, int bot, uint16_t time_ms);
void display_bcd(uint16_t top, uint16_t mid, uint16_t bot, uint16_t time_ms);
void display_number(uint16_t num, uint16_t time_ms);
void display_animate_slide(bool dir, uint32_t time);
void display_mono_set_color(uint8_t red, uint8_t green, uint8_t blue);

void display_string(char* string, uint16_t repeat, uint16_t scrollspeed);

#endif
