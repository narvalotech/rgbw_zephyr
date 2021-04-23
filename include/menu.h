#ifndef MENU_H_
#define MENU_H_

#define STYLE_MOVEMENT 1
#define STYLE_2D_TAPS  2

typedef struct
{
	int8_t x;
	int8_t y;
	int8_t z;
} tap_2d_struct_t;

typedef struct
{
	uint8_t single_tap;
	uint8_t double_tap;
} tap_struct_t;

uint8_t
itemSelector(char* itemNames, uint8_t inputStyle);

uint32_t
numberSelector(uint16_t defaultNum, uint16_t startNum, uint16_t endNum, uint8_t displayType);

#endif
