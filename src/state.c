#include <stdint.h>
#include <stdbool.h>
#include "state.h"

extern struct g_state state;

void state_clear(void)
{
	/* Clear all state except main/hibernate */
	state.abort_disp = 0;
	state.exit_signal = 0;
	state.select = 0;
	state.but_ll = 0;
	state.but_lr = 0;
	state.but_ur = 0;
}
