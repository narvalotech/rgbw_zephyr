#ifndef __STATE_H_
#define __STATE_H_

struct g_state {
	bool main;		/* Go back to main screen */
	bool exit_signal;	/* Exit current activity */
	bool abort_disp;	/* Abort scrolling string display */
	bool hibernate;		/* Hibernate ASAP */
	bool select;		/* User has selected current item */
};

#endif // __STATE_H_
