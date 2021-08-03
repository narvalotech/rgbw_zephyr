#include <zephyr.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "rgb_led.h"
#include "disp.h"
#include "alphabet.h"
#include "state.h"

typedef struct
{
	bool pending;
	uint8_t fx_type;
	bool dir;
	uint32_t time;
} fade_next_t;

static uint8_t display[4];
static uint8_t disp_color[3];
extern rgb_led_string_config_t led_cfg;
static rgb_led_string_config_t* p_led_cfg = &led_cfg;
extern struct g_state state;
static fade_next_t fade;

static bool s_pixel_here(uint8_t col, uint8_t line)
{
	return (bool)((display[line] >> col) & 1);
}

uint16_t reverse_bits(uint16_t word, uint16_t length) {
	// Bitwise bit reversal function
	uint16_t r = 0; // Contains reversed word
	uint16_t i;

	if(length == 8)
		word &= 0xFF;
	else
		word &= (1<<length) - 1;

	for (i = length - 1; i > 0; i--)
	{
		r |= word & 0x01;
		r <<= 1;
		word >>= 1;
	}
	r |= word & 0x01;

	return r;
}

void display_init() {
	// Display state
	display[0] = 0x00; // Top
	display[1] = 0x00; // Bottom
	display[2] = 0x00; // Middle
	display[3] = 0xFF; // Intensity
}

/* Uses disp_color[] global variable */
void display_mono_led(uint8_t intensity, uint32_t led_pos)
{
	rgb_led_set_led_scale(p_led_cfg,
			      disp_color[0],
			      disp_color[1],
			      disp_color[2],
			      intensity, led_pos, false);
}

void display_mono_set_color(uint8_t red, uint8_t green, uint8_t blue)
{
	disp_color[0] = red;
	disp_color[1] = green;
	disp_color[2] = blue;
}

void display_refresh()
{
	uint32_t disp_word = display[0]
		+ (reverse_bits(display[1], 8) << 8)
		+ (display[2] << 16);

	rgb_led_set_all(p_led_cfg, 0, 0, 0, false);

	if(fade.pending)
	{
		fade.pending = 0;

		if(fade.fx_type==DISP_FX_SLIDE)
			display_animate_slide(fade.dir, fade.time);

		if((fade.fx_type==DISP_FX_FADE) && (fade.dir==DISP_FX_DIR_IN))
			display_fade(fade.dir, fade.time);
	}

	for(int i=0; i<24; i++)
	{
		if(disp_word & 1)
			display_mono_led(display[3], i);
		disp_word >>= 1;
	}

	rgb_led_write(p_led_cfg);
}

void display_clear() {
	display[3] = 255;

	if(fade.pending)
	{
		fade.pending = 0;

		if(fade.fx_type==DISP_FX_SLIDE)
			display_animate_slide(fade.dir, fade.time);

		if((fade.fx_type==DISP_FX_FADE) && (fade.dir==DISP_FX_DIR_OUT))
			display_fade(fade.dir, fade.time);
	}
	else
	{
		rgb_led_set_all(p_led_cfg, 0, 0, 0, true);
	}

	display[0] = 0;
	display[1] = 0;
	display[2] = 0;
}

void display_write_matrix(uint8_t p_matrix[8][3], bool fade, bool clear_state)
{
	static uint8_t active[8][3] = {0};

	rgb_led_set_all(p_led_cfg, 0, 0, 0, false);

	if(clear_state)
		memset(active, 0, sizeof(active));

	/* First row */
	for(int j=0; j<8; j++)
	{
		if(!active[j][0])
			active[j][0] = p_matrix[j][0];

		if((fade) && (active[j][0]) && (s_pixel_here(j, 0)))
			display_mono_led(255, j);
		else
			display_mono_led(p_matrix[j][0], j);
	}
	/* Second row (flipped) */
	for(int j=0; j<8; j++)
	{
		if(!active[j][1])
			active[j][1] = p_matrix[j][1];

		if((fade) && (active[j][1]) && (s_pixel_here(j, 1)))
			display_mono_led(255, 15-j);
		else
			display_mono_led(p_matrix[j][0], 15-j);
	}
	/* Last row */
	for(int j=0; j<8; j++)
	{
		if(!active[j][2])
			active[j][2] = p_matrix[j][2];

		if((fade) && (active[j][2]) && (s_pixel_here(j, 2)))
			display_mono_led(255, j+16);
		else
			display_mono_led(p_matrix[j][0], j+16);
	}

	rgb_led_write(p_led_cfg);
}

void display_bytes(int top, int mid, int bot, uint16_t time_ms) {
	display[0] = top & 0xFF;
	display[1] = mid & 0xFF;
	display[2] = bot & 0xFF;

	display_refresh();
	if(!time_ms) return;
	k_msleep(time_ms);
	display_clear();
}

void display_bcd(uint16_t top, uint16_t mid, uint16_t bot, uint16_t time_ms) {
	if(top>99) return;
	if(mid>99) return;
	if(bot>99) return;
	display[0] = ((top/10)<<4) + (top%10);
	display[1] = ((mid/10)<<4) + (mid%10);
	display[2] = ((bot/10)<<4) + (bot%10);

	display_refresh();
	if(!time_ms) return;
	k_msleep(time_ms);
	display_clear();
}

void display_number(uint16_t num, uint16_t time_ms) {
	if(num>99) return;
	display[0] = numbers[0][num % 10] + (numbers[0][num / 10]<<5);
	display[1] = numbers[1][num % 10] + (numbers[1][num / 10]<<5);
	display[2] = numbers[2][num % 10] + (numbers[2][num / 10]<<5);

	display_refresh();
	if(!time_ms) return;
	k_msleep(time_ms);
	display_clear();
}

void display_string(char* string, uint16_t repeat, uint16_t scrollspeed) {
	uint16_t i = 0;
	uint16_t j = 0;
	char currentChar = 0;

	/* TODO: figure out a cleaner way to do this */
	/* This resets the abort state so the user has to press
	 * the abort button once again during the display.
	 * Without this, the caller has to reset the state before
	 * calling this fn. */
	state.abort = 0;

	repeat++;
	while(repeat != 0) {
		repeat--;
		i = 0;
		j = 0;

		for(;string[j] != 0;j++) {

			currentChar = string[j];
			if(currentChar >= 'A' && currentChar <= 'Z') {
				/* Convert upper case to lower case */
				currentChar += 'a' - 'A';
			} else if(currentChar == ' ') {
				currentChar = 0x7B;
			} else if (currentChar >= '0' && currentChar <= '9') {
				/* Do nothing */
			} else if (currentChar < 'a' || currentChar > 'z') {
				/* Convert any other char to a dash */
				currentChar = 0x7C;
			}

			for(i=5;i>0;i--) {
				display[0] = (display[0]<<1);
				display[1] = (display[1]<<1);
				display[2] = (display[2]<<1);

				if(currentChar >= '0' && currentChar <= '9') {
					/* Handle numbers */
					display[0] += (numbers[0][currentChar - '0']>>(i-1)) & 1;
					display[1] += (numbers[1][currentChar - '0']>>(i-1)) & 1;
					display[2] += (numbers[2][currentChar - '0']>>(i-1)) & 1;
				} else {
					/* Handle letters */
					display[0] += (letters[0][currentChar - 'a']>>i) & 1;
					display[1] += (letters[1][currentChar - 'a']>>i) & 1;
					display[2] += (letters[2][currentChar - 'a']>>i) & 1;
				}
				display_refresh();
				k_msleep(scrollspeed);
				if(state.abort) return;
			}
		}

		for(j=2;j>0;j--) {
			currentChar = 0x7B;
			for(i=5;i>0;i--) {
				display[0] = (display[0]<<1);
				display[1] = (display[1]<<1);
				display[2] = (display[2]<<1);

				if(currentChar >= '0' && currentChar <= '9') {
					/* Handle numbers */
					display[0] += (numbers[0][currentChar - '0']>>(i-1)) & 1;
					display[1] += (numbers[1][currentChar - '0']>>(i-1)) & 1;
					display[2] += (numbers[2][currentChar - '0']>>(i-1)) & 1;
				} else {
					/* Handle letters */
					display[0] += (letters[0][currentChar - 'a']>>i) & 1;
					display[1] += (letters[1][currentChar - 'a']>>i) & 1;
					display[2] += (letters[2][currentChar - 'a']>>i) & 1;
				}
				display_refresh();
				k_msleep(scrollspeed);
				if(state.abort) return;
			}
		}
	}
}

void display_fade(bool dir, uint32_t fade_time)
{
	for(uint32_t i=0; i < fade_time; i++)
	{
		if(dir==DISP_FX_DIR_IN) /* Fade in */
			display[3] = (uint8_t)((255 * i) / fade_time);
		else
			display[3] = (uint8_t)(255 - ((255 * i) / fade_time));

		display_refresh();
		k_msleep(1);
	}

	display[3] = 255;
}

void display_fade_next(bool dir, uint32_t fade_time, uint8_t fx_type)
{
	fade.pending = 1;
	fade.dir = dir;
	fade.time = fade_time;
	fade.fx_type = fx_type;
}

void display_animate_slide(bool dir, uint32_t time)
{
	static uint8_t matrix[8][3] = {{0}};
	uint32_t idx = 0;

	time /= 16;

	if(dir == DISP_FX_DIR_LEFT)
	{
		/* Update first row */
		matrix[0][0] = 255;
		matrix[0][1] = 255;
		matrix[0][2] = 255;
		display_write_matrix(matrix, 1, 1);

		for(int j=1; j < 16; j++)
		{
			k_msleep(time);

			/* Head */
			if(j<8)
			{
				idx = j;
				matrix[idx][0] = 255;
				matrix[idx][1] = 255;
				matrix[idx][2] = 255;
			}
			/* Tail fade */
			else
			{
				if(matrix[idx][0] > 31)
				{
					matrix[idx][0] -= 31;
					matrix[idx][1] -= 31;
					matrix[idx][2] -= 31;
				}
			}

			/* Update previous rows */
			for(int k = idx-1; k>=0; k--)
			{
				if(matrix[k][0] > 31)
				{
					matrix[k][0] = matrix[k+1][0] - 31;
					matrix[k][1] = matrix[k+1][1] - 31;
					matrix[k][2] = matrix[k+1][2] - 31;
				}
				else
				{
					matrix[k][0] = 0;
					matrix[k][1] = 0;
					matrix[k][2] = 0;
				}
			}

			/* Update display */
			display_write_matrix(matrix, 1, 0);
		}
	}
	else
	{
		/* Update first row */
		matrix[7][0] = 255;
		matrix[7][1] = 255;
		matrix[7][2] = 255;
		display_write_matrix(matrix, 1, 1);

		for(int j=14; j >= 0; j--)
		{
			k_msleep(time);

			/* Head */
			if(j>7)
			{
				idx = j-8;
				matrix[idx][0] = 255;
				matrix[idx][1] = 255;
				matrix[idx][2] = 255;
			}

			/* Tail fade */
			else
			{
				if(matrix[idx][0] > 31)
				{
					matrix[idx][0] -= 31;
					matrix[idx][1] -= 31;
					matrix[idx][2] -= 31;
				}
			}

			/* Update previous rows */
			for(int k = idx+1; k<8; k++)
			{
				if(matrix[k][0] > 31)
				{
					matrix[k][0] = matrix[k-1][0] - 31;
					matrix[k][1] = matrix[k-1][1] - 31;
					matrix[k][2] = matrix[k-1][2] - 31;
				}
				else
				{
					matrix[k][0] = 0;
					matrix[k][1] = 0;
					matrix[k][2] = 0;
				}
			}

			/* Update display */
			display_write_matrix(matrix, 1, 0);
		}
	}
	display_clear();
}
