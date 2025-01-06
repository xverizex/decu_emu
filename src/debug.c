#include "debug.h"
#include <ncurses.h>

struct debugger debugger = {14, 4};

uint32_t
is_debug_on_line (uint16_t line, uint8_t *posx, uint8_t *count)
{
	uint16_t start = debugger.offset / 16;
	uint16_t end = (debugger.offset + debugger.count) / 16;

	if (line >= start && line <= end) {
		if (start != end) {
			uint16_t d_count = 16 - debugger.offset;
			*posx = ((line == start)? debugger.offset: 0);

			*count = d_count;
		} else {
			*posx = debugger.offset;
			*count = debugger.count;
		}
		return 0; // THIS SHOULD BE TRUE; IT JUST TESTING FOR FUTURE.
	}

	return 0;
}
