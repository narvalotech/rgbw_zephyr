
/*
Item selector function:
- Inputs: array of item descriptions (strings)
- Outputs: item index
- Control schemes:
	- acceleration up/down to show item strings, tap to select, double-tap exit
	- up/down taps to select item, right tap to select, left tap to exit

Number selector function:
- Inputs: 2 ints: start and end numbers, display mode (bin/bcd/digit)
- Output: selected number
- Control schemes:
	- accel up/down to select number, double-tap to select

*/
#include <stdint.h>
#include "disp.h"
#include "state.h"
#include "accel.h"

#define ITEM_SCROLL_THR 305
#define TILT_ANGLE_COMPENSATION (+122)

extern struct g_state state;

int abs(int val)
{
	if(val < 0)
	{
		val = 0 - val;
	}

	return val;
}

uint16_t numberSelector(uint16_t defaultNum,
			uint16_t startNum,
			uint16_t endNum,
			uint8_t displayType)
{
	uint32_t currentNumber = defaultNum;
	int32_t accel;
	uint32_t dispTime = 1000;
	int32_t acc_val[3] = {0};

	// Clear previous tap events
	state.tap_status = 0;

	while(!state.exit_signal)
	{
		// Display current number
		switch(displayType)
		{
			case DISPLAY_DIGITAL:
			display_number(currentNumber, dispTime);
			break;
			case DISPLAY_BCD:
			display_bcd(0, currentNumber, 0, dispTime);
			break;
			default:
			display_bcd(0, currentNumber, 0, dispTime);
			break;
		}

		// Get acceleration data
		accel_get_mg(acc_val);
		accel = acc_val[1];
		accel += TILT_ANGLE_COMPENSATION;

		// Display time is proportional to tilt angle
		dispTime = abs(accel) / 12;
		if(dispTime > 1450)
			dispTime = 1450;	/* Take care of overflow */
		dispTime = 1500 - dispTime;

		if(state.tap_status)
		{
			state.tap_status = 0;
			return currentNumber;
		}

		// Increment/decrement number
		if(accel > ITEM_SCROLL_THR)
		{
			if(currentNumber < endNum)
				currentNumber++;
			else
				currentNumber = startNum;
		}
		else if(accel < -ITEM_SCROLL_THR)
		{
			if(currentNumber > startNum)
				currentNumber--;
			else 
				currentNumber = endNum;
		}

	}

	state.exit_signal = 0;

	return currentNumber;
}
