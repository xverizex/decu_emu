#include "debug.h"
#include <ncurses.h>

uint32_t
is_debug_on_line (uint16_t line, uint8_t *posx, uint8_t *count)
{
	if (line == 0) {
		*posx = 2;
		*count = 4;
	}

	return 0;
}
