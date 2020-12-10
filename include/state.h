#ifndef __STATE_H_
#define __STATE_H_

struct g_state {
	bool exit_signal;	/* Exit current activity */
	bool abort_disp;	/* Abort scrolling string display */
	bool hibernate;		/* Hibernate ASAP */
	bool tap_status;	/* Accel has received TAP event */
	uint8_t button_pressed;	/* Pushbutton has been pressed */
};

#endif // __STATE_H_
