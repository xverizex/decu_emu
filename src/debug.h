#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>

#define COLOR_COMMON           1
#define COLOR_STEP_DEBUG       2

struct debugger {
	uint16_t offset;
	uint8_t count;
};

uint32_t is_debug_on_line (uint16_t line, uint8_t *posx, uint8_t *count);

#endif
